"""
Tablebase Testing Utility for UCI Chess Engines
================================================
Tests a UCI engine against Lichess tablebase API optimal moves.

Metrics:
  - Average DTM error (starting DTM vs achieved DTM after each engine move)
  - Blunders (engine plays into drawn/50-move-rule position from winning one)

Usage:
    python tb_test.py --engine ./your_engine --fen "8/8/8/8/8/8/1K1k4/1R6 w - - 0 1" --depth 16
    python tb_test.py --engine ./your_engine --fen-file fens.txt --depth 16

This script is made by Gioviok
"""

import argparse
import chess
import chess.engine
import chess.pgn
import requests
import time
import sys
from dataclasses import dataclass, field
from typing import Optional

# --- Lichess Tablebase API ---

API_BASE = "https://tablebase.lichess.ovh"
# Lichess asks for at most ~1 req/sec from scripts
MIN_REQUEST_INTERVAL = 1.1


class RateLimiter:
    """
    Enforces a minimum interval between HTTP requests by sleeping only
    the time *remaining* since the last request — so engine think time
    (or any other elapsed time) counts toward the interval.
    """
    def __init__(self, min_interval: float = MIN_REQUEST_INTERVAL):
        self.min_interval = min_interval
        self._last_request_time: float = 0.0

    def wait(self):
        elapsed = time.monotonic() - self._last_request_time
        remaining = self.min_interval - elapsed
        if remaining > 0:
            time.sleep(remaining)

    def mark(self):
        """Call immediately after a request completes."""
        self._last_request_time = time.monotonic()

    def wait_and_mark(self):
        self.wait()
        self.mark()


# Module-level limiter shared across all calls in a run
_rate_limiter = RateLimiter()


def query_tablebase(fen: str) -> Optional[dict]:
    """Query Lichess tablebase API for a position. Returns raw JSON or None on error."""
    url = f"{API_BASE}/standard"
    _rate_limiter.wait_and_mark()
    try:
        resp = requests.get(url, params={"fen": fen}, timeout=10)
        resp.raise_for_status()
        return resp.json()
    except requests.RequestException as e:
        print(f"  [API error] {e}", file=sys.stderr)
        return None


def get_tb_info(fen: str) -> Optional[dict]:
    """
    Returns a dict with:
      category: 'win' | 'draw' | 'loss' | 'cursed-win' | 'blessed-loss' | 'unknown'
      dtm:      int | None   (distance to mate, negative = losing side)
      dtz:      int | None   (distance to zero / 50-move clock reset)
      best_move: str | None  (UCI best move from tablebase)
    """
    data = query_tablebase(fen)
    if data is None:
        return None

    best_uci = None
    moves = data.get("moves", [])
    if moves:
        best_uci = moves[0].get("uci")

    return {
        "category": data.get("category", "unknown"),
        "dtm": data.get("dtm"),       # None if not available (>5 pieces etc.)
        "dtz": data.get("dtz"),
        "best_move": best_uci,
    }


# --- DTM helpers ---

def is_winning(category: str) -> bool:
    return category in ("win", "cursed-win")

def is_drawn(category: str) -> bool:
    return category in ("draw", "cursed-win", "blessed-loss")

def is_blunder(before_cat: str, after_cat: str) -> bool:
    """
    A blunder is: engine was in a winning position, then after its move
    the position (for the opponent) is no longer a loss — i.e. it's drawn/win for them.
    We check from the OPPONENT's perspective after the engine's move.
    before_cat: category of position BEFORE engine moves (engine to move, should be 'win')
    after_cat:  category of position AFTER engine moves (opponent to move)
    A win for the engine = opponent is in 'loss'. If after_cat != 'loss', engine blundered.
    """
    if not is_winning(before_cat):
        return False  # wasn't winning, can't blunder a win
    # after engine's move, opponent should be in 'loss'. Anything else is a blunder.
    return after_cat not in ("loss",)


# --- Engine wrapper ---

def get_engine_move(engine: chess.engine.SimpleEngine, board: chess.Board, depth: int) -> Optional[chess.Move]:
    try:
        # engine.protocol.send_line("ucinewgame")
        result = engine.play(board, chess.engine.Limit(depth=depth), info=chess.engine.INFO_ALL)
        # print(result.info)
        return result.move
    except chess.engine.EngineError as e:
        print(f"  [Engine error] {e}", file=sys.stderr)
        return None


# --- Core testing function ---

@dataclass
class GameResult:
    start_fen: str
    start_dtm: Optional[int]
    moves_played: int = 0
    dtm_errors: list = field(default_factory=list)   # |optimal_dtm - achieved_dtm| per move
    blunders: list = field(default_factory=list)      # list of FENs where blunder occurred
    terminal_category: str = "unknown"
    notes: list = field(default_factory=list)


def test_position(
    engine: chess.engine.SimpleEngine,
    start_fen: str,
    depth: int = 16,
    max_moves: int = 200,
    verbose: bool = True,
) -> GameResult:
    """
    Play a position from start_fen:
      - Engine plays its side at the given depth
      - Tablebase plays the optimal reply for the opposing side
      - Track DTM error and blunders after each engine move
    """
    board = chess.Board(start_fen)
    result = GameResult(start_fen=start_fen, start_dtm=None)

    if verbose:
        print(f"\n{'='*60}")
        print(f"Position: {start_fen}")

    # --- Get starting TB info ---
    tb_start = get_tb_info(start_fen)

    if tb_start is None:
        result.notes.append("Could not query tablebase for start position.")
        return result

    result.start_dtm = tb_start.get("dtm")
    start_cat = tb_start.get("category", "unknown")

    if verbose:
        print(f"Start category: {start_cat}, DTM: {result.start_dtm}")

    if not is_winning(start_cat):
        result.notes.append(f"Start position is not a win ({start_cat}), skipping.")
        if verbose:
            print(f"  Skipping: not a winning position for the side to move.")
        return result

    # Engine always plays the side to move in the start position
    engine_color = board.turn

    move_count = 0
    while not board.is_game_over() and move_count < max_moves:
        current_fen = board.fen()
        print(current_fen)

        if board.turn == engine_color:
            # --- Engine's turn ---
            tb_before = get_tb_info(current_fen)

            if tb_before is None:
                result.notes.append(f"Move {move_count}: TB query failed before engine move.")
                break

            cat_before = tb_before.get("category", "unknown")
            dtm_before = tb_before.get("dtm")

            # Optimal DTM from here: should decrease by 1 each ply
            # After the engine moves, opponent will be in a 'loss' position
            # with DTM = dtm_before - 1 (in absolute terms)
            optimal_remaining = abs(dtm_before) if dtm_before is not None else None

            engine_move = get_engine_move(engine, board, depth)
            if engine_move is None:
                result.notes.append(f"Move {move_count}: Engine returned no move.")
                break

            if verbose:
                print(f"  Move {move_count+1} [Engine/{chess.COLOR_NAMES[engine_color]}]: {board.san(engine_move)}", end="")

            board.push(engine_move)
            move_count += 1
            result.moves_played = move_count

            # --- Query TB after engine move ---
            tb_after = get_tb_info(board.fen())

            if tb_after is None:
                result.notes.append(f"Move {move_count}: TB query failed after engine move.")
                if verbose:
                    print()
                break

            cat_after = tb_after.get("category", "unknown")
            dtm_after = tb_after.get("dtm")  # from opponent's perspective

            # DTM error: optimal was dtm_before-1, engine achieved dtm_after
            # Both are "moves to mate" for the winning side.
            # After engine moves: opponent is mated in |dtm_after| more moves.
            # Optimal: opponent should be mated in |dtm_before| - 1 moves.
            if optimal_remaining is not None and dtm_after is not None:
                # dtm_after is from opponent's POV (they're losing), so it's negative
                achieved = abs(dtm_after)
                optimal  = optimal_remaining - 1  # one ply consumed
                error = achieved - optimal         # >0 means engine was slower
                result.dtm_errors.append(error)
                if verbose:
                    print(f"  | cat={cat_after}, DTM_opt={optimal}, DTM_got={achieved}, err={error:+d}", end="")
            else:
                if verbose:
                    print(f"  | cat={cat_after}, DTM unavailable", end="")

            # Blunder check
            if is_blunder(cat_before, cat_after):
                result.blunders.append(current_fen)
                if verbose:
                    print(f"  *** BLUNDER: {cat_before} -> {cat_after} ***", end="")

            if verbose:
                print()

            result.terminal_category = cat_after

            if board.is_game_over():
                break

        else:
            # --- Tablebase's turn (optimal reply) ---
            tb_now = get_tb_info(board.fen())

            if tb_now is None or tb_now.get("best_move") is None:
                result.notes.append(f"Move {move_count}: No TB move available for opponent.")
                break

            tb_move = chess.Move.from_uci(tb_now["best_move"])
            if tb_move not in board.legal_moves:
                result.notes.append(f"Move {move_count}: TB move {tb_now['best_move']} is illegal.")
                break

            if verbose:
                print(f"  Move {move_count+1} [TB/{chess.COLOR_NAMES[not engine_color]}]:    {board.san(tb_move)}")

            board.push(tb_move)
            move_count += 1
            result.moves_played = move_count

    if board.is_checkmate():
        result.terminal_category = "checkmate"
        if verbose:
            print(f"  Result: Checkmate in {move_count} moves.")
    elif board.is_game_over():
        result.terminal_category = board.result()
        if verbose:
            print(f"  Result: {board.result()} after {move_count} moves.")

    game = chess.pgn.Game.from_board(board)
    game.setup(board.root())
    with open("games.pgn", "a") as pgnout:
        print(game, file=pgnout)

    return result


# --- Multi-position batch runner ---

@dataclass
class BatchStats:
    total_positions: int = 0
    skipped: int = 0
    total_moves: int = 0
    all_dtm_errors: list = field(default_factory=list)
    total_blunders: int = 0
    blunder_positions: list = field(default_factory=list)


def run_batch(
    engine_path: str,
    fens: list[str],
    depth: int = 16,
    max_moves: int = 200,
    verbose: bool = True,
) -> BatchStats:
    stats = BatchStats(total_positions=len(fens))

    with chess.engine.SimpleEngine.popen_uci(engine_path) as engine:
        for i, fen in enumerate(fens):
            print(f"\n[{i+1}/{len(fens)}] Testing: {fen}")
            result = test_position(engine, fen, depth=depth, max_moves=max_moves, verbose=verbose)

            if result.notes:
                for note in result.notes:
                    print(f"  NOTE: {note}")

            if not result.dtm_errors and not result.blunders and result.moves_played == 0:
                stats.skipped += 1
                continue

            stats.total_moves += result.moves_played
            stats.all_dtm_errors.extend(result.dtm_errors)
            stats.total_blunders += len(result.blunders)
            stats.blunder_positions.extend(result.blunders)

    return stats


def print_summary(stats: BatchStats):
    tested = stats.total_positions - stats.skipped
    print(f"\n{'='*60}")
    print(f"SUMMARY")
    print(f"{'='*60}")
    print(f"Positions tested : {tested} / {stats.total_positions} ({stats.skipped} skipped)")
    print(f"Total moves      : {stats.total_moves}")

    if stats.all_dtm_errors:
        avg_err = sum(stats.all_dtm_errors) / len(stats.all_dtm_errors)
        max_err = max(stats.all_dtm_errors)
        nonzero = sum(1 for e in stats.all_dtm_errors if e > 0)
        print(f"\nDTM Error (moves slower than optimal):")
        print(f"  Average error  : {avg_err:.2f} moves")
        print(f"  Max error      : {max_err} moves")
        print(f"  Suboptimal moves: {nonzero} / {len(stats.all_dtm_errors)}")
    else:
        print(f"\nDTM Error: N/A (DTM unavailable for all positions)")

    print(f"\nBlunders (winning -> drawn/lost):")
    print(f"  Total blunders : {stats.total_blunders}")
    if stats.blunder_positions:
        print(f"  Blunder FENs:")
        for fen in stats.blunder_positions:
            print(f"    {fen}")


# --- CLI ---

def main():
    parser = argparse.ArgumentParser(description="Test UCI engine against Lichess tablebases")
    parser.add_argument("--engine", required=True, help="Path to UCI engine binary")
    parser.add_argument("--fen", help="Single FEN to test")
    parser.add_argument("--fen-file", help="File with one FEN per line")
    parser.add_argument("--depth", type=int, default=16, help="Search depth (default: 16)")
    parser.add_argument("--max-moves", type=int, default=200, help="Max moves per game (default: 200)")
    parser.add_argument("--quiet", action="store_true", help="Suppress per-move output")
    args = parser.parse_args()

    fens = []
    if args.fen:
        fens.append(args.fen)
    if args.fen_file:
        with open(args.fen_file) as f:
            fens.extend(line.strip() for line in f if line.strip() and not line.startswith("#"))

    if not fens:
        parser.error("Provide --fen or --fen-file")

    stats = run_batch(
        engine_path=args.engine,
        fens=fens,
        depth=args.depth,
        max_moves=args.max_moves,
        verbose=not args.quiet,
    )
    print_summary(stats)


if __name__ == "__main__":
    main()
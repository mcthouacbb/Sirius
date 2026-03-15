"""
Tablebase Testing Utility for UCI Chess Engines
================================================
Tests a UCI engine against tablebase-optimal play.

Supports two TB backends:
  --tb-lichess          Query Lichess API (default, no local files needed)
  --tb-local <path>     Use local Syzygy files via python-chess (fast, no rate limit)

Metrics:
  - Average DTM error (starting DTM vs actual DTM achieved by engine)
  - Blunders (engine entering drawn/50-move-rule position from a winning one)

Usage:
    python tb_test.py --engine ./eng --fen "<fen>" --depth 16
    python tb_test.py --engine ./eng --fen "<fen>" --depth 16 --tb-local /path/to/syzygy
    python tb_test.py --engine ./eng --fen-file fens.txt --tb-local /path/to/syzygy

Thanks to Gioviok for the script
"""

import argparse
import abc
import chess
import chess.engine
import chess.gaviota
import chess.pgn
import chess.syzygy
import requests
import time
import sys
from dataclasses import dataclass, field
from typing import Optional


# ---------------------------------------------------------------------------
# TB backend abstraction
# ---------------------------------------------------------------------------

@dataclass
class TBInfo:
    category:   str             # 'win' | 'loss' | 'draw' | 'cursed-win' | 'blessed-loss' | 'unknown'
    dtm:        Optional[int]   # None when unavailable
    dtz:        Optional[int]
    best_move:  Optional[str]   # UCI string


class TBBackend(abc.ABC):
    @abc.abstractmethod
    def get_info(self, board: chess.Board) -> Optional[TBInfo]:
        """Return TBInfo for the position, or None on failure."""
        ...

    def close(self):
        pass


# ---------------------------------------------------------------------------
# Lichess HTTP backend
# ---------------------------------------------------------------------------

API_BASE = "https://tablebase.lichess.ovh"
MIN_REQUEST_INTERVAL = 1.1


class RateLimiter:
    """
    Enforces a minimum interval between HTTP requests by sleeping only
    the time *remaining* since the last request — so engine think time
    (or any other elapsed wall time) counts toward the interval.
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
        self._last_request_time = time.monotonic()

    def wait_and_mark(self):
        self.wait()
        self.mark()


class LichessBackend(TBBackend):
    def __init__(self):
        self._limiter = RateLimiter()

    def get_info(self, board: chess.Board) -> Optional[TBInfo]:
        fen = board.fen()
        self._limiter.wait_and_mark()
        try:
            resp = requests.get(f"{API_BASE}/standard", params={"fen": fen}, timeout=10)
            resp.raise_for_status()
            data = resp.json()
        except requests.RequestException as e:
            print(f"  [Lichess TB error] {e}", file=sys.stderr)
            return None

        moves = data.get("moves", [])
        best_uci = moves[0].get("uci") if moves else None

        return TBInfo(
            category=data.get("category", "unknown"),
            dtm=data.get("dtm"),
            dtz=data.get("dtz"),
            best_move=best_uci,
        )


# ---------------------------------------------------------------------------
# Local Syzygy backend
# ---------------------------------------------------------------------------

# DTZ wdl -> category mapping (from the side to move's perspective)
_WDL_TO_CATEGORY = {
    2:  "win",
    1:  "cursed-win",
    0:  "draw",
    -1: "blessed-loss",
    -2: "loss",
}


class LocalSyzygyBackend(TBBackend):
    """
    Uses python-chess's built-in Syzygy reader.

    Notes:
      - Syzygy files provide DTZ, not DTM. DTM errors will be reported as
        unavailable; blunder detection uses category only (which is exact).
      - The best move is chosen by probing all legal moves and picking the one
        that maximises progress (lowest DTZ for wins, highest for losses/draws).
    """

    def __init__(self, path: str):
        try:
            self._tb = chess.syzygy.open_tablebase(path)
        except Exception as e:
            print(f"[Local TB] Failed to open tablebase at '{path}': {e}", file=sys.stderr)
            raise

    def _probe(self, board: chess.Board) -> Optional[tuple[int, int]]:
        """Returns (wdl, dtz) for the position, or None on probe failure."""
        try:
            wdl = self._tb.probe_wdl(board)
            dtz = self._tb.probe_dtz(board)
            return wdl, dtz
        except chess.syzygy.MissingTableError:
            return None

    def get_info(self, board: chess.Board) -> Optional[TBInfo]:
        result = self._probe(board)
        if result is None:
            return None
        wdl, dtz = result
        category = _WDL_TO_CATEGORY.get(wdl, "unknown")

        # Find the best move by probing each legal move's resulting position
        best_move = self._best_move(board, wdl)

        return TBInfo(
            category=category,
            dtm=None,   # Syzygy doesn't provide DTM
            dtz=dtz,
            best_move=best_move,
        )

    def _best_move(self, board: chess.Board, current_wdl: int) -> Optional[str]:
        """
        Pick the TB-optimal move:
          - If winning (wdl > 0): find move that gives opponent the worst WDL
            (most negative), breaking ties by lowest abs(dtz) to convert fastest.
          - If losing (wdl < 0): maximise own survival (highest opponent WDL,
            i.e. slowest loss).
          - If drawing: pick any move that keeps the draw.
        """
        best_uci = None
        best_score = None   # (opponent_wdl, dtz_tiebreak) — lower is better for wins

        for move in board.legal_moves:
            board.push(move)
            try:
                probe = self._probe(board)
            finally:
                board.pop()

            if probe is None:
                continue
            opp_wdl, opp_dtz = probe

            if current_wdl > 0:
                # winning: minimise opponent's wdl, then minimise abs(dtz) to mate fast
                score = (opp_wdl, abs(opp_dtz))
                if best_score is None or score < best_score:
                    best_score = score
                    best_uci = move.uci()
            elif current_wdl < 0:
                # losing: maximise opponent's wdl (drag it out)
                score = (-opp_wdl, -abs(opp_dtz))
                if best_score is None or score < best_score:
                    best_score = score
                    best_uci = move.uci()
            else:
                # drawing: stay drawn
                if opp_wdl == 0:
                    best_uci = move.uci()
                    break

        return best_uci

    def close(self):
        self._tb.close()


# ---------------------------------------------------------------------------
# Local Gaviota backend (exact DTM)
# ---------------------------------------------------------------------------

class LocalGaviotaBackend(TBBackend):
    """
    Uses python-chess's Gaviota interface for exact DTM.

    No external library required — python-chess includes a pure-Python
    fallback prober that works out of the box with just the .gtb.cp4 files.
    If libgtb is available it will be used automatically for better speed;
    pass its path explicitly via --tb-gaviota-lib if needed.

    Download .gtb.cp4 files (MIT licensed, ~7 GB for all 5-piece endings):
      https://archive.org/details/Gaviota
    """

    def __init__(self, path: str, libgtb_path: Optional[str] = None):
        try:
            self._tb = chess.gaviota.open_tablebase(path, libgtb=libgtb_path)
        except Exception as e:
            print(f"[Gaviota TB] Failed to open tablebase at '{path}': {e}", file=sys.stderr)
            raise
        mode = "native (libgtb)" if type(self._tb).__name__ == "NativeTablebase" else "pure Python"
        print(f"  Gaviota probing mode: {mode}")

    def _probe_dtm(self, board: chess.Board) -> Optional[int]:
        """
        Returns DTM in half-moves from the side-to-move's perspective:
          positive = STM is winning (mated in N plies)
          negative = STM is losing
          0        = draw or already mated
        Returns None if the position isn't in the tablebase.
        """
        try:
            return self._tb.probe_dtm(board)
        except chess.gaviota.MissingTableError:
            return None

    def get_info(self, board: chess.Board) -> Optional[TBInfo]:
        dtm = self._probe_dtm(board)
        if dtm is None:
            return None

        # DTM sign directly encodes the outcome — no separate WDL probe needed
        if dtm > 0:
            category = "win"
        elif dtm < 0:
            category = "loss"
        else:
            category = "draw"

        best_move = self._best_move(board, dtm)

        return TBInfo(
            category=category,
            dtm=dtm,
            dtz=None,
            best_move=best_move,
        )

    def _best_move(self, board: chess.Board, current_dtm: int) -> Optional[str]:
        """
        Pick the TB-optimal move using DTM directly:
          - Winning (dtm > 0): find move where opponent has the most negative
            DTM — i.e. they are mated soonest from their own perspective.
          - Losing  (dtm < 0): find move where opponent has the highest DTM
            — i.e. drag out our own loss as long as possible.
          - Drawing (dtm == 0): find any move that keeps DTM == 0.
        """
        best_uci  = None
        best_score = None

        for move in board.legal_moves:
            board.push(move)
            try:
                opp_dtm = self._probe_dtm(board)
            finally:
                board.pop()

            if opp_dtm is None:
                continue

            if current_dtm > 0:
                # Minimise opponent's DTM (most negative = fastest mate for us)
                if best_score is None or opp_dtm < best_score:
                    best_score = opp_dtm
                    best_uci = move.uci()
            elif current_dtm < 0:
                # Maximise opponent's DTM (slowest loss for us)
                if best_score is None or opp_dtm > best_score:
                    best_score = opp_dtm
                    best_uci = move.uci()
            else:
                # Stay drawn
                if opp_dtm == 0:
                    best_uci = move.uci()
                    break

        return best_uci

    def close(self):
        self._tb.close()


def make_backend(args) -> TBBackend:
    if args.tb_syzygy:
        print(f"Using local Syzygy tablebases at: {args.tb_syzygy}  (DTZ only, no DTM)")
        return LocalSyzygyBackend(args.tb_syzygy)
    if args.tb_gaviota:
        libgtb = getattr(args, "tb_gaviota_lib", None)
        print(f"Using local Gaviota tablebases at: {args.tb_gaviota}  (exact DTM)")
        return LocalGaviotaBackend(args.tb_gaviota, libgtb_path=libgtb)
    print("Using Lichess tablebase API  (DTM available for ≤5 pieces)")
    return LichessBackend()


# ---------------------------------------------------------------------------
# Shared helpers
# ---------------------------------------------------------------------------

def get_tb_info(backend: TBBackend, board: chess.Board) -> Optional[TBInfo]:
    return backend.get_info(board)


def is_winning(category: str) -> bool:
    return category in ("win", "cursed-win")

def is_drawn(category: str) -> bool:
    return category in ("draw", "cursed-win", "blessed-loss")

def is_blunder(before_cat: str, after_cat: str) -> bool:
    """
    Engine was winning before its move; after the move the opponent is no
    longer in a losing position — the win has been thrown away.
    """
    if not is_winning(before_cat):
        return False
    return after_cat not in ("loss")


# ---------------------------------------------------------------------------
# Engine wrapper
# ---------------------------------------------------------------------------

def get_engine_move(engine: chess.engine.SimpleEngine, board: chess.Board, depth: int) -> Optional[chess.Move]:
    try:
        result = engine.play(board, chess.engine.Limit(depth=depth))
        return result.move
    except chess.engine.EngineError as e:
        print(f"  [Engine error] {e}", file=sys.stderr)
        return None


# ---------------------------------------------------------------------------
# Core testing function
# ---------------------------------------------------------------------------

@dataclass
class GameResult:
    start_fen: str
    start_dtm: Optional[int]
    moves_played: int = 0
    dtm_errors: list = field(default_factory=list)
    blunders: list = field(default_factory=list)      # FENs where blunder occurred
    terminal_category: str = "unknown"
    notes: list = field(default_factory=list)


def test_position(
    engine: chess.engine.SimpleEngine,
    backend: TBBackend,
    start_fen: str,
    depth: int = 16,
    max_moves: int = 200,
    verbose: bool = True,
    pgnout: str = "games.pgn"
) -> GameResult:
    board = chess.Board(start_fen)
    result = GameResult(start_fen=start_fen, start_dtm=None)

    if verbose:
        print(f"\n{'='*60}")
        print(f"Position: {start_fen}")

    tb_start = get_tb_info(backend, board)
    if tb_start is None:
        result.notes.append("Could not query tablebase for start position.")
        return result

    result.start_dtm = tb_start.dtm
    start_cat = tb_start.category

    if verbose:
        print(f"Start category: {start_cat}, DTM: {result.start_dtm}")

    if not is_winning(start_cat):
        result.notes.append(f"Start position is not a win ({start_cat}), skipping.")
        if verbose:
            print(f"  Skipping: not a winning position for the side to move.")
        return result

    engine_color = board.turn
    move_count = 0

    while not board.is_game_over() and move_count < max_moves:
        current_fen = board.fen()

        if board.turn == engine_color:
            # --- Engine's turn ---
            tb_before = get_tb_info(backend, board)
            if tb_before is None:
                result.notes.append(f"Move {move_count}: TB query failed before engine move.")
                break

            cat_before = tb_before.category
            dtm_before = tb_before.dtm
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

            tb_after = get_tb_info(backend, board)
            if tb_after is None:
                result.notes.append(f"Move {move_count}: TB query failed after engine move.")
                if verbose:
                    print()
                break

            cat_after = tb_after.category
            dtm_after = tb_after.dtm

            if optimal_remaining is not None and dtm_after is not None:
                achieved = abs(dtm_after)
                optimal  = optimal_remaining - 1
                error    = achieved - optimal
                result.dtm_errors.append(error)
                if verbose:
                    print(f"  | cat={cat_after}, DTM_opt={optimal}, DTM_got={achieved}, err={error:+d}", end="")
            else:
                if verbose:
                    print(f"  | cat={cat_after}, DTM unavailable", end="")

            if is_blunder(cat_before, cat_after) and not board.is_checkmate():
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
            tb_now = get_tb_info(backend, board)
            if tb_now is None or tb_now.best_move is None:
                result.notes.append(f"Move {move_count}: No TB move available for opponent.")
                break

            tb_move = chess.Move.from_uci(tb_now.best_move)
            if tb_move not in board.legal_moves:
                result.notes.append(f"Move {move_count}: TB move {tb_now.best_move} is illegal.")
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

    game = chess.pgn.Game()
    game.setup(chess.Board(start_fen))
    game.headers["White"] = "Engine" if chess.Board(start_fen).turn == chess.WHITE else "TB"
    game.headers["Black"] = "Engine" if chess.Board(start_fen).turn == chess.BLACK else "TB"

    node = game
    for move in board.move_stack:
        node = node.add_variation(move)

    with open(pgnout, "a") as pgnout_file:
        print(game, file=pgnout_file)
        print("\n", file=pgnout_file)

    return result


# ---------------------------------------------------------------------------
# Batch runner
# ---------------------------------------------------------------------------

@dataclass
class BatchStats:
    total_positions: int = 0
    skipped: int = 0
    total_moves: int = 0
    all_dtm_errors: list = field(default_factory=list)
    total_blunders: int = 0
    blunder_positions: list = field(default_factory=list)
    converted_positions: int = 0


def run_batch(
    engine_path: str,
    backend: TBBackend,
    fens: list[str],
    depth: int = 16,
    max_moves: int = 200,
    verbose: bool = True,
    pgnout: str = "games.pgn",
) -> BatchStats:
    stats = BatchStats(total_positions=len(fens))

    with chess.engine.SimpleEngine.popen_uci(engine_path) as engine:
        for i, fen in enumerate(fens):
            print(f"\n[{i+1}/{len(fens)}] Testing: {fen}")
            result = test_position(engine, backend, fen,
                                   depth=depth, max_moves=max_moves, verbose=verbose, pgnout=pgnout)

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
            if result.terminal_category == "checkmate":
                stats.converted_positions += 1

    return stats


def print_summary(stats: BatchStats):
    tested = stats.total_positions - stats.skipped
    print(f"\n{'='*60}")
    print(f"SUMMARY")
    print(f"{'='*60}")
    print(f"Positions tested    : {tested} / {stats.total_positions} ({stats.skipped} skipped)")
    print(f"Positions converted : {stats.converted_positions} / {stats.total_positions - stats.skipped}")
    print(f"Total moves         : {stats.total_moves}")

    if stats.all_dtm_errors:
        avg_err = sum(stats.all_dtm_errors) / len(stats.all_dtm_errors)
        max_err = max(stats.all_dtm_errors)
        nonzero = sum(1 for e in stats.all_dtm_errors if e > 0)
        print(f"\nDTM Error (moves slower than optimal):")
        print(f"  Average error   : {avg_err:.2f} moves")
        print(f"  Max error       : {max_err} moves")
        print(f"  Suboptimal moves: {nonzero} / {len(stats.all_dtm_errors)}")
    else:
        print(f"\nDTM Error: N/A (DTM unavailable — local Syzygy only provides DTZ)")

    print(f"\nBlunders (winning -> drawn/lost):")
    print(f"  Total blunders : {stats.total_blunders}")
    if stats.blunder_positions:
        print(f"  Blunder FENs:")
        for fen in stats.blunder_positions:
            print(f"    {fen}")


# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(description="Test UCI engine against tablebases")

    parser.add_argument("--engine",   required=True, help="Path to UCI engine binary")
    parser.add_argument("--fen",      help="Single FEN to test")
    parser.add_argument("--fen-file", help="File with one FEN per line")
    parser.add_argument("--depth",    type=int, default=16, help="Search depth (default: 16)")
    parser.add_argument("--max-moves",type=int, default=200, help="Max moves per game (default: 200)")
    parser.add_argument("--quiet",    action="store_true", help="Suppress per-move output")

    tb_group = parser.add_mutually_exclusive_group()
    tb_group.add_argument("--tb-lichess", action="store_true", default=True,
                          help="Use Lichess API — default (rate-limited ~1 req/s, DTM for ≤5 pieces)")
    tb_group.add_argument("--tb-syzygy", metavar="PATH",
                          help="Use local Syzygy files (fast, no rate limit; DTZ only, no DTM)")
    tb_group.add_argument("--tb-gaviota", metavar="PATH",
                          help="Use local Gaviota .gtb.cp4 files (exact DTM, pure Python — no libgtb needed)")
    parser.add_argument("--tb-gaviota-lib", metavar="PATH",
                        help="Optional: path to libgtb.so/.dylib to speed up Gaviota probing")
    parser.add_argument("--pgnout", default="games.pgn", help="Name of the pgn file to output")

    args = parser.parse_args()

    fens = []
    if args.fen:
        fens.append(args.fen)
    if args.fen_file:
        with open(args.fen_file) as f:
            fens.extend(line.strip() for line in f if line.strip() and not line.startswith("#"))

    if not fens:
        parser.error("Provide --fen or --fen-file")

    backend = make_backend(args)
    try:
        stats = run_batch(
            engine_path=args.engine,
            backend=backend,
            fens=fens,
            depth=args.depth,
            max_moves=args.max_moves,
            verbose=not args.quiet,
            pgnout=args.pgnout
        )
    finally:
        backend.close()

    print_summary(stats)


if __name__ == "__main__":
    main()
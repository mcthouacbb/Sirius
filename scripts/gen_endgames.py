import chess
import chess.syzygy
import random
import argparse
from typing import Optional, Tuple

_syzygy_tb = None

# expects pieces of the form "K*vK*"
def get_material_dist(pieces: str) -> list[chess.Piece]:
    [left, right] = pieces.split("v")

    PIECE_TYPE_MAP = {
        'P': chess.PAWN,
        'N': chess.KNIGHT,
        'B': chess.BISHOP,
        'R': chess.ROOK,
        'Q': chess.QUEEN,
    }

    result = [chess.Piece(chess.KING, chess.WHITE), chess.Piece(chess.KING, chess.BLACK)]
    for chr in left.removeprefix("K"):
        if not chr in PIECE_TYPE_MAP:
            raise ValueError(f"{chr} is not valid")
        result.append(chess.Piece(PIECE_TYPE_MAP[chr], chess.WHITE))

    for chr in right.removeprefix("K"):
        if not chr in PIECE_TYPE_MAP:
            raise ValueError(f"{chr} is not valid")
        result.append(chess.Piece(PIECE_TYPE_MAP[chr], chess.BLACK))

    return result

def gen_random_pos_single(pieces: list[chess.Piece]) -> Optional[chess.Board]:
    squares = []

    def gen_rand_square_unique():
        while True:
            new_square = random.randint(0, 63)
            if not new_square in squares:
                squares.append(new_square)
                return new_square

    board = chess.Board("8/8/8/8/8/8/8/8 w - - 0 1")
    for piece in pieces:
        square = gen_rand_square_unique()
        board.set_piece_at(chess.Square(square), piece)

    if not board.is_valid():
        return None
    return board

def gen_random_pos(pieces: list[chess.Piece]) -> chess.Board:
    while True:
        board = gen_random_pos_single(pieces)
        if not board is None:
            return board

def probe_wdl_dtz(pos: chess.Board) -> Tuple[int, int]:
    wdl = _syzygy_tb.probe_wdl(pos)
    dtz = _syzygy_tb.probe_dtz(pos)
    return wdl, dtz

def gen_random_pos_constrained(pieces: list[chess.Piece], min_dtz: int, max_dtz: int) -> chess.Board:
    while True:
        board = gen_random_pos(pieces)
        wdl, dtz = probe_wdl_dtz(board)
        if wdl != 2:
            continue
        if dtz < min_dtz or dtz > max_dtz:
            continue
        return board


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("mat_dist", type=str, help="The material distribution to generate (e.g. KRvKN). Advantaged side must be first")
    parser.add_argument("syzygy_path", type=str, help="Path to syzygy tablebases")
    parser.add_argument("--min-dtz", type=int, default=10, help="The minimum DTZ to generate")
    parser.add_argument("--max-dtz", type=int, default=100, help="The maximum DTZ to generate")
    parser.add_argument("--pos-count", type=int, default=1, help="Number of positions to generate")

    args = parser.parse_args()

    global _syzygy_tb
    _syzygy_tb = chess.syzygy.open_tablebase(args.syzygy_path)

    mat_dist = get_material_dist(args.mat_dist)
    for i in range(args.pos_count):
        print(gen_random_pos_constrained(mat_dist, args.min_dtz, args.max_dtz).fen())

if __name__ == "__main__":
    main()

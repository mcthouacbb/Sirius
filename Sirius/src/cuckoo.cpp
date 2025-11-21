#include "cuckoo.h"
#include "attacks.h"
#include "zobrist.h"

namespace cuckoo
{

// implementation based on Integral's https://github.com/aronpetko/integral/commit/3c09d3fcff3bb520fca58608c41ffcf7832534f6
// and Stormphrax's https://github.com/Ciekce/Stormphrax/commit/a0ee2dac3e4fe27870a6a2ef7e1567f2345a64da
// and Stockfish's implementation https://github.com/official-stockfish/Stockfish/blob/master/src/position.cpp
// and the original paper https://web.archive.org/web/20201107002606/https://marcelk.net/2013-04-06/paper/upcoming-rep-v2.pdf

void init()
{
    moves.fill(Move::nullmove());
    keyDiffs.fill(0);

    u32 count = 0;

    using enum Color;
    using enum PieceType;

    for (PieceType pt : {KNIGHT, BISHOP, ROOK, QUEEN, KING})
    {
        for (Color c : {WHITE, BLACK})
        {
            for (i32 from = 0; from < 63; from++)
            {
                for (i32 to = from + 1; to < 64; to++)
                {
                    Bitboard pieceAttacks;
                    switch (pt)
                    {
                        case PieceType::KNIGHT:
                            pieceAttacks = attacks::knightAttacks(Square(from));
                            break;
                        case PieceType::BISHOP:
                            pieceAttacks = attacks::bishopAttacks(Square(from), EMPTY_BB);
                            break;
                        case PieceType::ROOK:
                            pieceAttacks = attacks::rookAttacks(Square(from), EMPTY_BB);
                            break;
                        case PieceType::QUEEN:
                            pieceAttacks = attacks::queenAttacks(Square(from), EMPTY_BB);
                            break;
                        case PieceType::KING:
                            pieceAttacks = attacks::kingAttacks(Square(from));
                            break;
                    }

                    if (!pieceAttacks.has(Square(to)))
                        continue;

                    auto move = Move(Square(from), Square(to), MoveType::NONE);
                    ZKey zkey = {};
                    zkey.addPiece(pt, c, Square(from));
                    zkey.addPiece(pt, c, Square(to));
                    zkey.flipSideToMove();

                    u64 keyDiff = zkey.value;

                    u32 slot = H1(keyDiff);

                    while (true)
                    {
                        std::swap(keyDiffs[slot], keyDiff);
                        std::swap(moves[slot], move);

                        if (move == Move::nullmove())
                            break;

                        slot = slot == H1(keyDiff) ? H2(keyDiff) : H1(keyDiff);
                    }

                    count++;
                }
            }
        }
    }

    assert(count == 3668);
}

}

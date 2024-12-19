#include "cuckoo.h"
#include "zobrist.h"
#include "attacks.h"

namespace cuckoo
{

// implementation based on https://github.com/aronpetko/integral/commit/3c09d3fcff3bb520fca58608c41ffcf7832534f6
// and https://web.archive.org/web/20201107002606/https://marcelk.net/2013-04-06/paper/upcoming-rep-v2.pdf

void init()
{
	moves.fill(Move());
	keyDiffs.fill(0);

	uint32_t count = 0;

    using enum Color;
    using enum PieceType;

    for (PieceType pt : {KNIGHT, BISHOP, ROOK, QUEEN, KING})
    {
        for (Color c : {WHITE, BLACK})
        {
            for (int from = 0; from < 63; from++)
            {
                for (int to = from + 1; to < 64; to++)
                {
                    Bitboard pieceAttacks;
                    switch (pt)
                    {
                        case PieceType::KNIGHT:
                            pieceAttacks = attacks::knightAttacks(Square(from));
                            break;
                        case PieceType::BISHOP:
                            pieceAttacks = attacks::bishopAttacks(Square(from), Bitboard(0));
                            break;
                        case PieceType::ROOK:
                            pieceAttacks = attacks::rookAttacks(Square(from), Bitboard(0));
                            break;
                        case PieceType::QUEEN:
                            pieceAttacks = attacks::queenAttacks(Square(from), Bitboard(0));
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

                    uint64_t keyDiff = zkey.value;

                    uint32_t slot = H1(keyDiff);

                    while (true)
                    {
                        std::swap(keyDiffs[slot], keyDiff);
                        std::swap(moves[slot], move);

                        if (move == Move())
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

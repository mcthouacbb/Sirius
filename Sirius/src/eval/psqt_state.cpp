#include "psqt_state.h"
#include "../board.h"

namespace eval
{

void PsqtState::refresh(const Board& board)
{
    materialPsqt = PackedScore(0, 0);
    for (Color c : {Color::WHITE, Color::BLACK})
    {
        buckets[static_cast<int>(c)] = getKingBucket(board.kingSq(c));
        for (PieceType pt : {PieceType::PAWN, PieceType::KNIGHT, PieceType::BISHOP, PieceType::ROOK, PieceType::QUEEN, PieceType::KING})
        {
            Bitboard pieces = board.pieces(c, pt);
            while (pieces.any())
            {
                uint32_t sq = pieces.poplsb();
                PackedScore d = combinedPsqtScore(buckets[static_cast<int>(c)], c, pt, sq);
                materialPsqt += d;
            }
        }
    }
    needsRefresh = false;
}


}

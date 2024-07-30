#include "psqt_state.h"
#include "../board.h"

namespace eval
{

void Accumulator::refresh(const Board& board, Color c)
{
    materialPsqt = PackedScore(0, 0);
    bucket = getKingBucket(board.kingSq(c));
    for (PieceType pt : {PieceType::PAWN, PieceType::KNIGHT, PieceType::BISHOP, PieceType::ROOK, PieceType::QUEEN, PieceType::KING})
    {
        Bitboard pieces = board.pieces(c, pt);
        while (pieces.any())
        {
            uint32_t sq = pieces.poplsb();
            PackedScore d = combinedPsqtScore(bucket, c, pt, sq);
            materialPsqt += d;
        }
    }
    needsRefresh = false;
}

PackedScore PsqtState::evaluate(const Board& board) const
{
    if (accumulators[static_cast<int>(Color::WHITE)].needsRefresh)
        accumulators[static_cast<int>(Color::WHITE)].refresh(board, Color::WHITE);
    if (accumulators[static_cast<int>(Color::BLACK)].needsRefresh)
        accumulators[static_cast<int>(Color::BLACK)].refresh(board, Color::BLACK);

    return accumulators[static_cast<int>(Color::WHITE)].materialPsqt + accumulators[static_cast<int>(Color::BLACK)].materialPsqt;
}


}

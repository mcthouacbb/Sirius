#include "phase.h"

namespace eval
{

constexpr int PHASE_WEIGHTS[] = {
    // pawn
    PAWN_PHASE,
    // knight
    KNIGHT_PHASE,
    // bishop
    BISHOP_PHASE,
    // rook
    ROOK_PHASE,
    // queen
    QUEEN_PHASE,
    // king
    0
};

int getPiecePhase(PieceType piece)
{
    return PHASE_WEIGHTS[static_cast<int>(piece)];
}

int getFullEval(int mg, int eg, int phase)
{
    phase = std::min(phase, TOTAL_PHASE);
    return (mg * (TOTAL_PHASE - phase) + eg * phase) / TOTAL_PHASE;
}

}

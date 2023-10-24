#pragma once

#include "../defs.h"

namespace eval
{

constexpr int PAWN_PHASE = 0;
constexpr int KNIGHT_PHASE = 1;
constexpr int BISHOP_PHASE = 1;
constexpr int ROOK_PHASE = 2;
constexpr int QUEEN_PHASE = 4;

constexpr int TOTAL_PHASE =
    PAWN_PHASE * 16 +
    KNIGHT_PHASE * 4 +
    BISHOP_PHASE * 4 +
    ROOK_PHASE * 4 +
    QUEEN_PHASE * 2;

int getPiecePhase(PieceType piece);
int getFullEval(int mg, int eg, int phase);

}

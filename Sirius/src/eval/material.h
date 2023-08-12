#pragma once

#include "../defs.h"

namespace eval
{

constexpr int QUEEN_VALUE_MG = 858;
constexpr int ROOK_VALUE_MG = 418;
constexpr int BISHOP_VALUE_MG = 320;
constexpr int KNIGHT_VALUE_MG = 286;
constexpr int PAWN_VALUE_MG = 63;

constexpr int QUEEN_VALUE_EG = 703;
constexpr int ROOK_VALUE_EG = 377;
constexpr int BISHOP_VALUE_EG = 208;
constexpr int KNIGHT_VALUE_EG = 189;
constexpr int PAWN_VALUE_EG = 107;


int getPieceValueMG(PieceType piece);
int getPieceValueEG(PieceType piece);

}
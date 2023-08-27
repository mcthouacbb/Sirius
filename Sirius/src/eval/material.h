#pragma once

#include "../defs.h"

namespace eval
{

constexpr int QUEEN_VALUE_MG = 1029;
constexpr int ROOK_VALUE_MG = 492;
constexpr int BISHOP_VALUE_MG = 374;
constexpr int KNIGHT_VALUE_MG = 335;
constexpr int PAWN_VALUE_MG = 74;

constexpr int QUEEN_VALUE_EG = 942;
constexpr int ROOK_VALUE_EG = 506;
constexpr int BISHOP_VALUE_EG = 286;
constexpr int KNIGHT_VALUE_EG = 258;
constexpr int PAWN_VALUE_EG = 99;


int getPieceValueMG(PieceType piece);
int getPieceValueEG(PieceType piece);

}
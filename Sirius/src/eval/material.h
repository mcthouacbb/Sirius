#pragma once

#include "../defs.h"

namespace eval
{

constexpr int QUEEN_VALUE_MG = 2006;
constexpr int ROOK_VALUE_MG = 904;
constexpr int BISHOP_VALUE_MG = 703;
constexpr int KNIGHT_VALUE_MG = 638;
constexpr int PAWN_VALUE_MG = 176;

constexpr int QUEEN_VALUE_EG = 1821;
constexpr int ROOK_VALUE_EG = 1027;
constexpr int BISHOP_VALUE_EG = 567;
constexpr int KNIGHT_VALUE_EG = 515;
constexpr int PAWN_VALUE_EG = 256;


int getPieceValueMG(PieceType piece);
int getPieceValueEG(PieceType piece);

}
#pragma once

#include "../defs.h"

namespace eval
{

constexpr int QUEEN_VALUE_MG = 1993;
constexpr int ROOK_VALUE_MG = 932;
constexpr int BISHOP_VALUE_MG = 734;
constexpr int KNIGHT_VALUE_MG = 668;
constexpr int PAWN_VALUE_MG = 180;

constexpr int QUEEN_VALUE_EG = 1882;
constexpr int ROOK_VALUE_EG = 1033;
constexpr int BISHOP_VALUE_EG = 568;
constexpr int KNIGHT_VALUE_EG = 515;
constexpr int PAWN_VALUE_EG = 257;


int getPieceValueMG(PieceType piece);
int getPieceValueEG(PieceType piece);

}
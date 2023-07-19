#pragma once

#include "../defs.h"

namespace eval
{

constexpr int QUEEN_VALUE_MG = 900;
constexpr int ROOK_VALUE_MG = 500;
constexpr int BISHOP_VALUE_MG = 330;
constexpr int KNIGHT_VALUE_MG = 320;
constexpr int PAWN_VALUE_MG = 100;

constexpr int QUEEN_VALUE_EG = 900;
constexpr int ROOK_VALUE_EG = 500;
constexpr int BISHOP_VALUE_EG = 330;
constexpr int KNIGHT_VALUE_EG = 320;
constexpr int PAWN_VALUE_EG = 100;


int getPieceValueMG(PieceType piece);
int getPieceValueEG(PieceType piece);

}
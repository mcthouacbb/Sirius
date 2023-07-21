#pragma once

#include "../defs.h"

namespace eval
{

constexpr int QUEEN_VALUE_MG = 794;
constexpr int ROOK_VALUE_MG = 370;
constexpr int BISHOP_VALUE_MG = 291;
constexpr int KNIGHT_VALUE_MG = 265;
constexpr int PAWN_VALUE_MG = 70;

constexpr int QUEEN_VALUE_EG = 749;
constexpr int ROOK_VALUE_EG = 411;
constexpr int BISHOP_VALUE_EG = 225;
constexpr int KNIGHT_VALUE_EG = 204;
constexpr int PAWN_VALUE_EG = 101;


int getPieceValueMG(PieceType piece);
int getPieceValueEG(PieceType piece);

}
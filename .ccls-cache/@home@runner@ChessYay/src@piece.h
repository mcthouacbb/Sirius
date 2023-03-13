#pragma once

#include "bitboard.h"

enum class Color
{
	WHITE,
	BLACK
};

inline Color getOppColor(Color c)
{
	return c == Color::WHITE ? Color::BLACK : Color::WHITE;
}

enum class PieceType
{
	PAWN,
	KING,
	QUEEN,
	ROOK,
	BISHOP,
	KNIGHT,
	NONE
};

constexpr size_t NUM_PIECES = 6;
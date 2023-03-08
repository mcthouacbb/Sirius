#pragma once

#include <cstdint>
#include <iostream>
#include <bitset>
#include <bit>

enum class PieceType
{
	ALL = 0,
	NONE = 0,
	KING,
	QUEEN,
	ROOK,
	BISHOP,
	KNIGHT,
	PAWN
};

using Piece = uint8_t;

constexpr int PIECE_NONE = 0;
constexpr int PIECE_TYPE_MASK = 0b111;
constexpr int PIECE_COL_MASK = 0b1000;

enum class Color
{
	WHITE,
	BLACK
};

inline Color flip(Color c)
{
	return static_cast<Color>(static_cast<int>(c) ^ 1);
}

template<Color c>
inline constexpr Color flip()
{
	return static_cast<Color>(static_cast<int>(c) ^ 1);
}
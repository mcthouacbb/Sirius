#pragma once

#include <cstdint>
#include <iostream>
#include <bitset>
#include <bit>
#include <cassert>

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


enum class MoveType
{
	NONE = 0 << 12,
	ENPASSANT = 1 << 12,
	PROMOTION = 2 << 12,
	CASTLE = 3 << 12
};

enum class Promotion
{
	QUEEN = 0 << 14,
	ROOK = 1 << 14,
	BISHOP = 2 << 14,
	KNIGHT = 3 << 14
};

struct Move
{
public:
	Move() = default;
	Move(int src, int dst, MoveType type);
	Move(int src, int dst, MoveType type, Promotion promotion);

	int srcPos() const;
	int dstPos() const;
	MoveType type() const;
	Promotion promotion() const;
private:
	static constexpr int TYPE_MASK = 3 << 12;
	static constexpr int PROMOTION_MASK = 3 << 14;
	uint16_t m_Data;
};

inline Move::Move(int src, int dst, MoveType type)
	: m_Data(0)
{
	assert(src >= 0 && src < 64 && "Src pos is out of range");
	assert(dst >= 0 && dst < 64 && "Dst pos is out of range");
	m_Data = static_cast<uint16_t>(src | (dst << 6) | static_cast<int>(type));
}

inline Move::Move(int src, int dst, MoveType type, Promotion promotion)
	: m_Data(0)
{
	assert(src >= 0 && src < 64 && "Src pos is out of range");
	assert(dst >= 0 && dst < 64 && "Dst pos is out of range");
	m_Data = static_cast<uint16_t>(src | (dst << 6) | static_cast<int>(type) | static_cast<int>(promotion));
}


inline int Move::srcPos() const
{
	return m_Data & 63;
}

inline int Move::dstPos() const
{
	return (m_Data >> 6) & 63;
}

inline MoveType Move::type() const
{
	return static_cast<MoveType>(m_Data & TYPE_MASK);
}

inline Promotion Move::promotion() const
{
	return static_cast<Promotion>(m_Data & PROMOTION_MASK);
}
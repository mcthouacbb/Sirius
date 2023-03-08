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

enum class Piece : uint8_t {};

constexpr Piece PIECE_NONE = Piece(0);

inline Piece makePiece(PieceType type, Color color)
{
	return Piece((static_cast<int>(color) << 3) | static_cast<int>(type));
}

inline PieceType getPieceType(Piece piece)
{
	return static_cast<PieceType>(static_cast<int>(piece) & 0b111);
}

inline Color getPieceColor(Piece piece)
{
	return static_cast<Color>(static_cast<int>(piece) >> 3);
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

inline PieceType promoPiece(Promotion promo)
{
	static const PieceType promoPieces[4] = {
		PieceType::QUEEN,
		PieceType::ROOK,
		PieceType::BISHOP,
		PieceType::KNIGHT
	};

	return promoPieces[static_cast<int>(promo) >> 14];
}

struct Move
{
public:
	Move() = default;
	Move(int src, int dst, MoveType type);
	Move(int src, int dst, MoveType type, Promotion promotion);

	bool operator==(const Move& other) const = default;
	bool operator!=(const Move& other) const = default;

	int srcPos() const;
	int dstPos() const;
	int fromTo() const;
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

inline int Move::fromTo() const
{
	return m_Data & 4095;
}

inline MoveType Move::type() const
{
	return static_cast<MoveType>(m_Data & TYPE_MASK);
}

inline Promotion Move::promotion() const
{
	return static_cast<Promotion>(m_Data & PROMOTION_MASK);
}
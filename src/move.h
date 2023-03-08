#pragma once

#include "defs.h"

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
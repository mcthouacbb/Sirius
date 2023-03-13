#pragma once

#include <cstdint>

#include "piece.h"

enum class MoveType : uint32_t
{
	NONE,
	DOUBLE_PUSH,
	KSIDE_CASTLE,
	QSIDE_CASTLE,
	CAPTURE,
	PROMOTION,
	CAPTURE_PROMOTION,
	ENPASSANT
};

enum class CastleSide
{
	KING,
	QUEEN
};

struct NullMove
{

};



struct Move
{
public:
	Move() = default;
	Move(NullMove);
	Move(MoveType type, uint32_t srcPos, uint32_t dstPos, PieceType srcPieceType);
	Move(MoveType type, uint32_t srcPos, uint32_t dstPos, PieceType srcPieceType, PieceType dstPieceType);

	MoveType type() const;
	uint32_t srcPos() const;
	uint32_t dstPos() const;
	PieceType srcPieceType() const;
	PieceType dstPieceType() const;
private:
	/*
	---------------------------------------------------------
	|  0-2  |  3-8   |  9-14  |    15-17     |    18-20     |
	| flags | srcPos | dstPos | srcPieceType | dstPieceType |
	---------------------------------------------------------
	*/
	uint32_t m_Data;
};

inline MoveType Move::type() const
{
	return static_cast<MoveType>(m_Data & 0x7);
}

inline uint32_t Move::srcPos() const
{
	return (m_Data >> 3) & 0x3F;
}

inline uint32_t Move::dstPos() const
{
	return (m_Data >> 9) & 0x3F;
}

inline PieceType Move::srcPieceType() const
{
	return static_cast<PieceType>((m_Data >> 15) & 0x7);
}

inline PieceType Move::dstPieceType() const
{
	return static_cast<PieceType>((m_Data >> 18) & 0x7);
}
#include <cassert>

#include "move.h"

Move::Move(NullMove)
	: m_Data(0)
{
}

Move::Move(MoveType type, uint32_t srcPos, uint32_t dstPos, PieceType srcPieceType)
	: m_Data(0)
{
	// assert((type & ~0x7) == 0 && "type occupies bits outside 0-2");
	assert((srcPos & ~0x3F) == 0 && "srcPos occupies bits outside 3-8");
	assert((dstPos & ~0x3F) == 0 && "srcPos occupies bits outside 9-14");
	// assert((srcPieceType & ~0x7) == 0 && "srcPieceType occupies bits outside 15-17");
	m_Data |= static_cast<uint32_t>(type);
	m_Data |= srcPos << 3;
	m_Data |= dstPos << 9;
	m_Data |= static_cast<uint32_t>(srcPieceType) << 15;
	m_Data |= static_cast<uint32_t>(PieceType::NONE) << 18;
}

Move::Move(MoveType type, uint32_t srcPos, uint32_t dstPos, PieceType srcPieceType, PieceType dstPieceType)
	: m_Data(0)
{
	// assert((type & ~0x7) == 0 && "type occupies bits outside 0-2");
	assert((srcPos & ~0x3F) == 0 && "srcPos occupies bits outside 3-8");
	assert((dstPos & ~0x3F) == 0 && "srcPos occupies bits outside 9-14");
	// assert((srcPieceType & ~0x7) == 0 && "srcPieceType occupies bits outside 15-17");
	// assert((dstPieceType & ~0x7) == 0 && "dstPieceType occupies bits outside of 18-20");
	m_Data |= static_cast<uint32_t>(type);
	m_Data |= srcPos << 3;
	m_Data |= dstPos << 9;
	m_Data |= static_cast<uint32_t>(srcPieceType) << 15;
	m_Data |= static_cast<uint32_t>(dstPieceType) << 18;
}
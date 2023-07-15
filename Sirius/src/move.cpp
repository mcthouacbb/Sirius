#include "move.h"
#include <cassert>

Move::Move(int src, int dst, MoveType type)
	: m_Data(0)
{
	assert(src >= 0 && src < 64 && "Src pos is out of range");
	assert(dst >= 0 && dst < 64 && "Dst pos is out of range");
	m_Data = static_cast<uint16_t>(src | (dst << 6) | static_cast<int>(type));
}

Move::Move(int src, int dst, MoveType type, Promotion promotion)
	: m_Data(0)
{
	assert(src >= 0 && src < 64 && "Src pos is out of range");
	assert(dst >= 0 && dst < 64 && "Dst pos is out of range");
	m_Data = static_cast<uint16_t>(src | (dst << 6) | static_cast<int>(type) | static_cast<int>(promotion));
}
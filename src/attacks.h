#pragma once

#include "defs.h"
#include "bitboard.h"

enum class Direction
{
	NORTH,
	SOUTH,
	EAST,
	WEST,
	NORTH_EAST,
	NORTH_WEST,
	SOUTH_EAST,
	SOUTH_WEST
};

namespace attacks
{

extern BitBoard inBetweenSquares[64][64];
extern BitBoard alignedSquares[64][64];
extern BitBoard moveMasks[64][64];

extern int castleMasks[64];

void init();

BitBoard getRay(uint32_t pos, Direction dir);

BitBoard getPawnAttacks(Color color, uint32_t pos);
BitBoard getKingAttacks(uint32_t pos);
BitBoard getKnightAttacks(uint32_t pos);

BitBoard getBishopAttacks(uint32_t pos, BitBoard blockers);
BitBoard getRookAttacks(uint32_t pos, BitBoard blockers);
BitBoard getQueenAttacks(uint32_t pos, BitBoard blockers);

template<Color c>
inline BitBoard getPawnBBEastAttacks(BitBoard pawns)
{
	if constexpr (c == Color::WHITE)
	{
		return shiftNorthEast(pawns);
	}
	else
	{
		return shiftSouthEast(pawns);
	}
}

template<Color c>
inline BitBoard getPawnBBWestAttacks(BitBoard pawns)
{
	if constexpr (c == Color::WHITE)
	{
		return shiftNorthWest(pawns);
	}
	else
	{
		return shiftSouthWest(pawns);
	}
}

template<Color c>
inline BitBoard getPawnBBAttacks(BitBoard pawns)
{
	return getPawnBBWestAttacks<c>(pawns) | getPawnBBEastAttacks<c>(pawns);
}

template<Color c>
inline BitBoard getPawnBBPushes(BitBoard pawns)
{
	if constexpr (c == Color::WHITE)
	{
		return shiftNorth(pawns);
	}
	else
	{
		return shiftSouth(pawns);
	}
}

template<Color c>
inline constexpr int pawnPushOffset()
{
	return c == Color::WHITE ? 8 : -8;
}

template<Color c>
inline constexpr Direction pawnEastCaptureDir()
{
	return c == Color::WHITE ? Direction::NORTH_EAST : Direction::SOUTH_EAST;
}

template<Color c>
inline constexpr Direction pawnWestCaptureDir()
{
	return c == Color::WHITE ? Direction::NORTH_WEST : Direction::SOUTH_WEST;
}

template<Color c>
inline constexpr BitBoard kscCheckSquares()
{
	if constexpr (c == Color::WHITE)
	{
		return 0x70;
	}
	else
	{
		return 0x7000000000000000;
	}
}

template<Color c>
inline constexpr BitBoard qscCheckSquares()
{
	if constexpr (c == Color::WHITE)
	{
		return 0x1C;
	}
	else
	{
		return 0x1C00000000000000;
	}
}

template<Color c>
inline constexpr BitBoard kscBlockSquares()
{
	if constexpr (c == Color::WHITE)
	{
		return 0x60;
	}
	else
	{
		return 0x6000000000000000;
	}
}

template<Color c>
inline constexpr BitBoard qscBlockSquares()
{
	if constexpr (c == Color::WHITE)
	{
		return 0xE;
	}
	else
	{
		return 0xE00000000000000;
	}
}

inline BitBoard inBetweenBB(uint32_t src, uint32_t dst)
{
	return inBetweenSquares[src][dst];
}

inline BitBoard alignedBB(uint32_t src, uint32_t dst)
{
	return alignedSquares[src][dst];
}

inline BitBoard moveMaskBB(uint32_t king, uint32_t checker)
{
	return moveMasks[king][checker];
}

inline int getCastleMask(uint32_t pos)
{
	return castleMasks[pos];
}

}
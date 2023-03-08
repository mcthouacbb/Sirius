#pragma once

#include <cstdint>
#include <iostream>
#include <bit>
#include <bitset>

#if defined(_MSC_VER)
#include <intrin.h>
#endif

using BitBoard = uint64_t;


constexpr BitBoard FILE_A = 0x0101010101010101;
constexpr BitBoard FILE_B = 0x0202020202020202;
constexpr BitBoard FILE_C = 0x0404040404040404;
constexpr BitBoard FILE_D = 0x0808080808080808;
constexpr BitBoard FILE_E = 0x1010101010101010;
constexpr BitBoard FILE_F = 0x2020202020202020;
constexpr BitBoard FILE_G = 0x4040404040404040;
constexpr BitBoard FILE_H = 0x8080808080808080;

constexpr BitBoard RANK_1 = 0x00000000000000FF;
constexpr BitBoard RANK_2 = 0x000000000000FF00;
constexpr BitBoard RANK_3 = 0x0000000000FF0000;
constexpr BitBoard RANK_4 = 0x00000000FF000000;
constexpr BitBoard RANK_5 = 0x000000FF00000000;
constexpr BitBoard RANK_6 = 0x0000FF0000000000;
constexpr BitBoard RANK_7 = 0x00FF000000000000;
constexpr BitBoard RANK_8 = 0xFF00000000000000;

template<Color c, int r>
constexpr BitBoard nthRank()
{
	if (c == Color::WHITE)
	{
		return RANK_1 << (8 * r);
	}
	else
	{
		return RANK_8 >> (8 * r);
	}
}


inline uint8_t reverse(uint8_t b) {
	 b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
	 b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
	 b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
	 return b;
}

inline void printBB(BitBoard bb)
{
	for (int i = 0; i < 8; i++)
	{
		uint8_t val = reverse(bb >> 56);
		std::cout << std::bitset<8>(val) << std::endl;
		bb <<= 8;
	}
	std::cout << std::endl;
}

inline BitBoard shiftNorth(BitBoard bb)
{
	return bb << 8;
}

inline BitBoard shiftSouth(BitBoard bb)
{
	return bb >> 8;
}

inline BitBoard shiftEast(BitBoard bb)
{
	return (bb << 1) & ~FILE_A;
}

inline BitBoard shiftWest(BitBoard bb)
{
	return (bb >> 1) & ~FILE_H;
}

inline BitBoard shiftNorthEast(BitBoard bb)
{
	return shiftNorth(shiftEast(bb));
}

inline BitBoard shiftNorthWest(BitBoard bb)
{
	return shiftNorth(shiftWest(bb));
}

inline BitBoard shiftSouthEast(BitBoard bb)
{
	return shiftSouth(shiftEast(bb));
}

inline BitBoard shiftSouthWest(BitBoard bb)
{
	return shiftSouth(shiftWest(bb));
}

inline uint32_t getLSB(BitBoard bb)
{
#if defined(__GNUC__)
	return __builtin_ctzll(bb);
#elif defined(_MSC_VER)
	unsigned long idx;
	_BitScanForward64(&idx, bb);
	return idx;
#endif
}

inline uint32_t getMSB(BitBoard bb)
{
#if defined(__GNUC__)
	return 63 - __builtin_clzll(bb);
#elif defined(_MSC_VER)
	unsigned long idx;
	_BitScanReverse64(&idx, bb);
	return idx;
#endif
}

inline uint32_t popLSB(BitBoard& bb)
{
	uint32_t lsb = getLSB(bb);
	bb &= bb - 1;
	return lsb;
}

inline uint32_t getPopcnt(BitBoard bb)
{
	return std::popcount(bb);
}
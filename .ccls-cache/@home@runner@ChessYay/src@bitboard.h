#pragma once

#include <cstdint>
#include <iostream>
#include <bitset>
#include <bit>

#if defined(_MSC_VER)
#include <intrin.h>
#endif


using BitBoard = uint64_t;

inline unsigned char reverse(unsigned char b) {
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

constexpr BitBoard ONE = 1ull;
constexpr BitBoard ALL = UINT64_MAX;

constexpr BitBoard files[8] = {
	0x0101010101010101,
	0x0202020202020202,
	0x0404040404040404,
	0x0808080808080808,
	0x1010101010101010,
	0x2020202020202020,
	0x4040404040404040,
	0x8080808080808080
};

constexpr BitBoard ranks[8] = {
	0x00000000000000FF,
	0x000000000000FF00,
	0x0000000000FF0000,
	0x00000000FF000000,
	0x000000FF00000000,
	0x0000FF0000000000,
	0x00FF000000000000,
	0xFF00000000000000
};

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
	return (bb << 1) & ~files[0];
}

inline BitBoard shiftWest(BitBoard bb)
{
	return (bb >> 1) & ~files[7];
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
/*#if defined(__GNUC__)
	return __builtin_popcountll(bb);
#elif defined(_MSC_VER)
	return __popcnt(bb);
#endif*/
}
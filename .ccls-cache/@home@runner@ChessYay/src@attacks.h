#include "bitboard.h"

enum Direction
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

constexpr BitBoard WKSC_CHECK_SQUARES = 0x70;
constexpr BitBoard WQSC_CHECK_SQUARES = 0x1C;
constexpr BitBoard BKSC_CHECK_SQUARES = 0x7000000000000000;
constexpr BitBoard BQSC_CHECK_SQUARES = 0x1C00000000000000;

constexpr BitBoard WKSC_BLOCK_SQUARES = 0x60;
constexpr BitBoard WQSC_BLOCK_SQUARES = 0xE;
constexpr BitBoard BKSC_BLOCK_SQUARES = 0x6000000000000000;
constexpr BitBoard BQSC_BLOCK_SQUARES = 0xE00000000000000;

BitBoard getRay(uint32_t idx, Direction dir);
void genAttackData();

BitBoard getBishopAttacks(uint32_t square, BitBoard blockers);
BitBoard getRookAttacks(uint32_t square, BitBoard blockers);
BitBoard getQueenAttacks(uint32_t square, BitBoard blockers);

BitBoard getDiagonalAttacks(uint32_t square, BitBoard blockers);
BitBoard getAntidiagonalAttacks(uint32_t square, BitBoard blockers);
BitBoard getVerticalAttacks(uint32_t square, BitBoard blockers);
BitBoard getHorizontalAttacks(uint32_t square, BitBoard blockers);

BitBoard getBishopAttacksSlow(uint32_t square, BitBoard blockers);
BitBoard getDiagonalAttacksSlow(uint32_t square, BitBoard blockers);
BitBoard getAntidiagonalAttacksSlow(uint32_t square, BitBoard blockers);

BitBoard getRookAttacksSlow(uint32_t square, BitBoard blockers);
BitBoard getVerticalAttacksSlow(uint32_t square, BitBoard blockers);
BitBoard getHorizontalAttacksSlow(uint32_t square, BitBoard blockers);

BitBoard getQueenAttacksSlow(uint32_t square, BitBoard blockers);

BitBoard getKnightAttacks(uint32_t square);
BitBoard getKingAttacks(uint32_t square);
#include "attacks.h"

namespace attacks
{

const uint64_t rookMagics[64] = {
  0xa8002c000108020ULL,
  0x6c00049b0002001ULL,
  0x100200010090040ULL,
  0x2480041000800801ULL,
  0x280028004000800ULL,
  0x900410008040022ULL,
  0x280020001001080ULL,
  0x2880002041000080ULL,
  0xa000800080400034ULL,
  0x4808020004000ULL,
  0x2290802004801000ULL,
  0x411000d00100020ULL,
  0x402800800040080ULL,
  0xb000401004208ULL,
  0x2409000100040200ULL,
  0x1002100004082ULL,
  0x22878001e24000ULL,
  0x1090810021004010ULL,
  0x801030040200012ULL,
  0x500808008001000ULL,
  0xa08018014000880ULL,
  0x8000808004000200ULL,
  0x201008080010200ULL,
  0x801020000441091ULL,
  0x800080204005ULL,
  0x1040200040100048ULL,
  0x120200402082ULL,
  0xd14880480100080ULL,
  0x12040280080080ULL,
  0x100040080020080ULL,
  0x9020010080800200ULL,
  0x813241200148449ULL,
  0x491604001800080ULL,
  0x100401000402001ULL,
  0x4820010021001040ULL,
  0x400402202000812ULL,
  0x209009005000802ULL,
  0x810800601800400ULL,
  0x4301083214000150ULL,
  0x204026458e001401ULL,
  0x40204000808000ULL,
  0x8001008040010020ULL,
  0x8410820820420010ULL,
  0x1003001000090020ULL,
  0x804040008008080ULL,
  0x12000810020004ULL,
  0x1000100200040208ULL,
  0x430000a044020001ULL,
  0x280009023410300ULL,
  0xe0100040002240ULL,
  0x200100401700ULL,
  0x2244100408008080ULL,
  0x8000400801980ULL,
  0x2000810040200ULL,
  0x8010100228810400ULL,
  0x2000009044210200ULL,
  0x4080008040102101ULL,
  0x40002080411d01ULL,
  0x2005524060000901ULL,
  0x502001008400422ULL,
  0x489a000810200402ULL,
  0x1004400080a13ULL,
  0x4000011008020084ULL,
  0x26002114058042ULL,
};

const uint64_t bishopMagics[64] = {
  0x89a1121896040240ULL,
  0x2004844802002010ULL,
  0x2068080051921000ULL,
  0x62880a0220200808ULL,
  0x4042004000000ULL,
  0x100822020200011ULL,
  0xc00444222012000aULL,
  0x28808801216001ULL,
  0x400492088408100ULL,
  0x201c401040c0084ULL,
  0x840800910a0010ULL,
  0x82080240060ULL,
  0x2000840504006000ULL,
  0x30010c4108405004ULL,
  0x1008005410080802ULL,
  0x8144042209100900ULL,
  0x208081020014400ULL,
  0x4800201208ca00ULL,
  0xf18140408012008ULL,
  0x1004002802102001ULL,
  0x841000820080811ULL,
  0x40200200a42008ULL,
  0x800054042000ULL,
  0x88010400410c9000ULL,
  0x520040470104290ULL,
  0x1004040051500081ULL,
  0x2002081833080021ULL,
  0x400c00c010142ULL,
  0x941408200c002000ULL,
  0x658810000806011ULL,
  0x188071040440a00ULL,
  0x4800404002011c00ULL,
  0x104442040404200ULL,
  0x511080202091021ULL,
  0x4022401120400ULL,
  0x80c0040400080120ULL,
  0x8040010040820802ULL,
  0x480810700020090ULL,
  0x102008e00040242ULL,
  0x809005202050100ULL,
  0x8002024220104080ULL,
  0x431008804142000ULL,
  0x19001802081400ULL,
  0x200014208040080ULL,
  0x3308082008200100ULL,
  0x41010500040c020ULL,
  0x4012020c04210308ULL,
  0x208220a202004080ULL,
  0x111040120082000ULL,
  0x6803040141280a00ULL,
  0x2101004202410000ULL,
  0x8200000041108022ULL,
  0x21082088000ULL,
  0x2410204010040ULL,
  0x40100400809000ULL,
  0x822088220820214ULL,
  0x40808090012004ULL,
  0x910224040218c9ULL,
  0x402814422015008ULL,
  0x90014004842410ULL,
  0x1000042304105ULL,
  0x10008830412a00ULL,
  0x2520081090008908ULL,
  0x40102000a0a60140ULL,
};

const uint32_t rookIndexBits[64] = {
	12, 11, 11, 11, 11, 11, 11, 12,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	12, 11, 11, 11, 11, 11, 11, 12
};

const uint32_t bishopIndexBits[64] = {
	6, 5, 5, 5, 5, 5, 5, 6,
	5, 5, 5, 5, 5, 5, 5, 5,
	5, 5, 7, 7, 7, 7, 5, 5,
	5, 5, 7, 9, 9, 7, 5, 5,
	5, 5, 7, 9, 9, 7, 5, 5,
	5, 5, 7, 7, 7, 7, 5, 5,
	5, 5, 5, 5, 5, 5, 5, 5,
	6, 5, 5, 5, 5, 5, 5, 6
};



BitBoard inBetweenSquares[64][64];
BitBoard pinRays[64][64];
BitBoard alignedSquares[64][64];
BitBoard moveMasks[64][64];



BitBoard rookAttacks[102400];
BitBoard bishopAttacks[5248];

BitBoard pawnAttacks[2][64] = {};
BitBoard kingAttacks[64] = {};
BitBoard knightAttacks[64] = {};

BitBoard rays[64][8] = {};

struct Magic
{
	BitBoard* attackData;
	BitBoard mask;
	uint64_t magic;
	uint32_t shift;
} bishopTable[64], rookTable[64];

int castleMasks[64] = {
	13, 15, 15, 15, 12, 15, 15, 14, // white
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	7, 15, 15, 15, 3, 15, 15, 11, // black
};

BitBoard getRay(uint32_t pos, Direction dir)
{
	return rays[pos][static_cast<uint32_t>(dir)];
}

inline BitBoard& rayFrom(uint32_t idx, Direction dir)
{
	return rays[idx][static_cast<uint32_t>(dir)];
}

BitBoard getMaskBlockerIdx(BitBoard mask, uint32_t idx)
{
	BitBoard blockers = 0;
	while (mask)
	{
		uint32_t lsb = popLSB(mask);
		if (idx & 1)
			blockers |= 1ull << lsb;
		idx >>= 1;
	}
	return blockers;
}

void initRays()
{
	for (uint32_t square = 0; square < 64; square++)
	{
		BitBoard bb = 1ull << square;

		BitBoard tmp = bb;
		BitBoard result = 0;
		while (tmp)
		{
			tmp = shiftNorth(tmp);
			result |= tmp;
		}
		rayFrom(square, Direction::NORTH) = result;

		tmp = bb;
		result = 0;
		while (tmp)
		{
			tmp = shiftSouth(tmp);
			result |= tmp;
		}
		rayFrom(square, Direction::SOUTH) = result;

		tmp = bb;
		result = 0;
		while (tmp)
		{
			tmp = shiftEast(tmp);
			result |= tmp;
		}
		rayFrom(square, Direction::EAST) = result;

		tmp = bb;
		result = 0;
		while (tmp)
		{
			tmp = shiftWest(tmp);
			result |= tmp;
		}
		rayFrom(square, Direction::WEST) = result;

		tmp = bb;
		result = 0;
		while (tmp)
		{
			tmp = shiftNorthEast(tmp);
			result |= tmp;
		}
		rayFrom(square, Direction::NORTH_EAST) = result;

		tmp = bb;
		result = 0;
		while (tmp)
		{
			tmp = shiftNorthWest(tmp);
			result |= tmp;
		}
		rayFrom(square, Direction::NORTH_WEST) = result;

		tmp = bb;
		result = 0;
		while (tmp)
		{
			tmp = shiftSouthEast(tmp);
			result |= tmp;
		}
		rayFrom(square, Direction::SOUTH_EAST) = result;

		tmp = bb;
		result = 0;
		while (tmp)
		{
			tmp = shiftSouthWest(tmp);
			result |= tmp;
		}
		rayFrom(square, Direction::SOUTH_WEST) = result;
	}
}

void initNonSlidingPieces()
{
	for (uint32_t square = 0; square < 64; square++)
	{
		BitBoard bb = 1ull << square;
		BitBoard king = shiftEast(bb) | shiftWest(bb);
		BitBoard lr = king | bb;
		king |= shiftNorth(lr) | shiftSouth(lr);

		kingAttacks[square] = king;

		BitBoard knight =
			shiftNorth(shiftNorthEast(bb)) |
			shiftNorth(shiftNorthWest(bb)) |
			shiftSouth(shiftSouthEast(bb)) |
			shiftSouth(shiftSouthWest(bb)) |
			shiftEast(shiftNorthEast(bb)) |
			shiftEast(shiftSouthEast(bb)) |
			shiftWest(shiftNorthWest(bb)) |
			shiftWest(shiftSouthWest(bb));
		knightAttacks[square] = knight;

		pawnAttacks[static_cast<int>(Color::WHITE)][square] = getPawnBBAttacks<Color::WHITE>(bb);
		pawnAttacks[static_cast<int>(Color::BLACK)][square] = getPawnBBAttacks<Color::BLACK>(bb);
	}
}

BitBoard getRookAttacksSlow(uint32_t square, BitBoard blockers)
{
	BitBoard ray = getRay(square, Direction::NORTH);
	BitBoard attacks = ray;
	if (BitBoard rayBlockers = ray & blockers)
		attacks ^= getRay(getLSB(rayBlockers), Direction::NORTH);

	ray = getRay(square, Direction::SOUTH);
	attacks |= ray;
	if (BitBoard rayBlockers = ray & blockers)
		attacks ^= getRay(getMSB(rayBlockers), Direction::SOUTH);

	ray = getRay(square, Direction::EAST);
	attacks |= ray;
	if (BitBoard rayBlockers = ray & blockers)
		attacks ^= getRay(getLSB(rayBlockers), Direction::EAST);

	ray = getRay(square, Direction::WEST);
	attacks |= ray;
	if (BitBoard rayBlockers = ray & blockers)
		attacks ^= getRay(getMSB(rayBlockers), Direction::WEST);
	return attacks;
}

BitBoard getBishopAttacksSlow(uint32_t square, BitBoard blockers)
{
	BitBoard ray = getRay(square, Direction::NORTH_EAST);
	BitBoard attacks = ray;
	if (BitBoard rayBlockers = ray & blockers)
		attacks ^= getRay(getLSB(rayBlockers), Direction::NORTH_EAST);

	ray = getRay(square, Direction::NORTH_WEST);
	attacks |= ray;
	if (BitBoard rayBlockers = ray & blockers)
		attacks ^= getRay(getLSB(rayBlockers), Direction::NORTH_WEST);

	ray = getRay(square, Direction::SOUTH_EAST);
	attacks |= ray;
	if (BitBoard rayBlockers = ray & blockers)
		attacks ^= getRay(getMSB(rayBlockers), Direction::SOUTH_EAST);

	ray = getRay(square, Direction::SOUTH_WEST);
	attacks |= ray;
	if (BitBoard rayBlockers = ray & blockers)
		attacks ^= getRay(getMSB(rayBlockers), Direction::SOUTH_WEST);
	return attacks;
}

void initSlidingPieces()
{
	constexpr BitBoard EDGE_SQUARES = FILE_A | FILE_H | RANK_1 | RANK_8;
	BitBoard* currRook = rookAttacks;
	BitBoard* currBishop = bishopAttacks;
	for (uint32_t square = 0; square < 64; square++)
	{
		BitBoard rookMask =
			(getRay(square, Direction::NORTH) & ~RANK_8) |
			(getRay(square, Direction::SOUTH) & ~RANK_1) |
			(getRay(square, Direction::EAST) & ~FILE_H) |
			(getRay(square, Direction::WEST) & ~FILE_A);

		rookTable[square].magic = rookMagics[square];
		rookTable[square].shift = 64 - rookIndexBits[square];
		rookTable[square].mask = rookMask;
		rookTable[square].attackData = currRook;

		for (uint32_t i = 0; i < (1u << rookIndexBits[square]); i++)
		{
			BitBoard blockers = getMaskBlockerIdx(rookMask, i);
			uint32_t idx = static_cast<uint32_t>((blockers * rookMagics[square]) >> (64 - rookIndexBits[square]));
			rookTable[square].attackData[idx] = getRookAttacksSlow(square, blockers);
			currRook++;
		}


		BitBoard bishopMask = ~EDGE_SQUARES &
			(getRay(square, Direction::NORTH_EAST) |
			getRay(square, Direction::NORTH_WEST) |
			getRay(square, Direction::SOUTH_EAST) |
			getRay(square, Direction::SOUTH_WEST));

		bishopTable[square].magic = bishopMagics[square];
		bishopTable[square].shift = 64 - bishopIndexBits[square];
		bishopTable[square].mask = bishopMask;
		bishopTable[square].attackData = currBishop;
		for (uint32_t i = 0; i < (1u << bishopIndexBits[square]); i++)
		{
			BitBoard blockers = getMaskBlockerIdx(bishopMask, i);
			uint32_t idx = static_cast<uint32_t>((blockers * bishopMagics[square]) >> (64 - bishopIndexBits[square]));

			bishopTable[square].attackData[idx] = getBishopAttacksSlow(square, blockers);
			currBishop++;
		}
	}
}

Direction oppDir(Direction d)
{
	switch (d)
	{
		case Direction::NORTH:
			return Direction::SOUTH;
		case Direction::SOUTH:
			return Direction::NORTH;
		case Direction::EAST:
			return Direction::WEST;
		case Direction::WEST:
			return Direction::EAST;
		case Direction::NORTH_EAST:
			return Direction::SOUTH_WEST;
		case Direction::NORTH_WEST:
			return Direction::SOUTH_EAST;
		case Direction::SOUTH_EAST:
			return Direction::NORTH_WEST;
		case Direction::SOUTH_WEST:
			return Direction::NORTH_EAST;
	}
	assert(false && "Invalid direction in oppDir");
	return Direction(-1);
}

void initBetweenBBs()
{
	Direction allDirs[] = {
		Direction::NORTH,
		Direction::SOUTH,
		Direction::EAST,
		Direction::WEST,
		Direction::NORTH_EAST,
		Direction::NORTH_WEST,
		Direction::SOUTH_EAST,
		Direction::SOUTH_WEST
	};
	for (uint32_t src = 0; src < 64; src++)
	{
		for (uint32_t dst = 0; dst < 64; dst++)
		{
			if (src == dst)
				continue;
			BitBoard dstBB = 1ull << dst;
			moveMasks[src][dst] = dstBB;
			for (Direction dir : allDirs)
			{
				BitBoard srcRay = getRay(src, dir);
				if (srcRay & dstBB)
				{
					BitBoard dstRay = getRay(dst, oppDir(dir));
					inBetweenSquares[src][dst] = srcRay & dstRay;
					pinRays[src][dst] = srcRay;
					moveMasks[src][dst] |= (srcRay & dstRay);
					alignedSquares[src][dst] = srcRay | dstRay;
					// printBB(moveMasks[src][dst]);
				}
			}
		}
	}
}


void init()
{
	initRays();
	initNonSlidingPieces();
	initSlidingPieces();
	initBetweenBBs();
}

BitBoard getPawnAttacks(Color color, uint32_t pos)
{
	return pawnAttacks[static_cast<int>(color)][pos];
}

BitBoard getKingAttacks(uint32_t pos)
{
	return kingAttacks[pos];
}

BitBoard getKnightAttacks(uint32_t pos)
{
	return knightAttacks[pos];
}

BitBoard getBishopAttacks(uint32_t pos, BitBoard blockers)
{
	blockers &= bishopTable[pos].mask;
	uint64_t index = blockers * bishopTable[pos].magic;
	return bishopTable[pos].attackData[index >> bishopTable[pos].shift];
}

BitBoard getRookAttacks(uint32_t pos, BitBoard blockers)
{
	blockers &= rookTable[pos].mask;
	uint64_t index = blockers * rookTable[pos].magic;
	return rookTable[pos].attackData[index >> rookTable[pos].shift];
}

BitBoard getQueenAttacks(uint32_t pos, BitBoard blockers)
{
	return getBishopAttacks(pos, blockers) | getRookAttacks(pos, blockers);
}


}

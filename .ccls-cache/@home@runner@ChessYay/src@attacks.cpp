#include "attacks.h"

BitBoard kingAttacks[64] = {};
BitBoard knightAttacks[64] = {};

BitBoard rays[64][8] = {};

// BitBoard rookAttacks[64][4096] = {};
// BitBoard bishopAttacks[64][512] = {};
BitBoard rookAttacks[102400];
BitBoard bishopAttacks[5248];

BitBoard rookMasks[64] = {};
BitBoard bishopMasks[64] = {};

/*const uint64_t rookMagics[64] = {
	0x211000a000808040ULL, 0x1840140004400020ULL, 0x10040205600008ULL, 0x300080108101001ULL,
	0x20a014040400408ULL, 0x801c020081040001ULL, 0x202452000200081ULL, 0x488000801021ULL,
	0x8004400000184020ULL, 0x61310004004040ULL, 0x10042090a080a0ULL, 0x8290001043088008ULL,
	0x5708204808008ULL, 0x400081e00010003ULL, 0x1a000c02400b0093ULL, 0x201209a005110201ULL,
	0x2203b00018004040ULL, 0xc010080120083420ULL, 0x9200410422600a08ULL, 0x4121882000404202ULL,
	0x6402040980404ULL, 0x6a80042081508081ULL, 0x2c005a4100101ULL, 0x1428014240101ULL,
	0xa0410c080a0092ULL, 0x462420000029102ULL, 0x8a14600164082501ULL, 0x1080440008240aULL,
	0x4008010200208415ULL, 0x420a044100100212ULL, 0x112c320020810007ULL, 0x42041200000c2ULL,
	0x104008280a284580ULL, 0x1040006010184020ULL, 0x100611204003201ULL, 0x10100200a100950ULL,
	0x80894080428024ULL, 0x40430000002284ULL, 0x40408100020013ULL, 0x4101028000001441ULL,
	0x7420800000009040ULL, 0x4008804000008020ULL, 0x1410018800104a8ULL, 0xc42040000001002ULL,
	0x91b200200080052aULL, 0x40400240000526ULL, 0x1008612081a81ULL, 0x6008080440a5401ULL,
	0xc182003b0100a2ULL, 0x100260400020041ULL, 0xa214100880104104ULL, 0x81000801c408138ULL,
	0xc02004002004024ULL, 0x4020800000822ULL, 0x9002010020402105ULL, 0x44008040014000e1ULL,
	0x4080944a20482a01ULL, 0x1298111220002046ULL, 0x408401183010020ULL, 0x1000060924002413ULL,
	0x42110902800004a2ULL, 0x2884215202000802ULL, 0x880810618421802ULL, 0x2800248308c010c1ULL,
};

const uint64_t bishopMagics[64] = {
	0x82004b10022146ULL, 0x2040810211443044ULL, 0x20a10500103102ULL, 0x1200029201041042ULL,
	0x2200100020210ULL, 0x540120022820804ULL, 0x844601400011808eULL, 0x210108000a111118ULL,
	0x9408820081001304ULL, 0x82a2084000009210ULL, 0x8040828802004818ULL, 0x8100100900342105ULL,
	0xe0204500848022aULL, 0x1041e01020382082ULL, 0x240424011400e208ULL, 0x1104310009800824ULL,
	0x20040082a01c0721ULL, 0x10402c4004c1022ULL, 0x400900a03120004ULL, 0x2c802893000200c1ULL,
	0x88d1000200020021ULL, 0x889088a01282102ULL, 0xc22020000416812ULL, 0x4a0690400480a1ULL,
	0x40900200444a9010ULL, 0x4890049002018411ULL, 0x8901020008407000ULL, 0x41006240204020ULL,
	0x1420218044004010ULL, 0x8284010218004104ULL, 0x49012840000c004aULL, 0x22080221014440ULL,
	0xe10c2200700844ULL, 0x9084040401409010ULL, 0x24010000005241ULL, 0x3110090008800401ULL,
	0x4801110200208044ULL, 0x120952011802087aULL, 0x2b10803010881ULL, 0x4020a022028206ULL,
	0x200212c080180206ULL, 0x1400092402144101ULL, 0x81001200080220a4ULL, 0x80011004800341ULL,
	0x48101140400c031ULL, 0x880010824010150ULL, 0x410108001142c9ULL, 0x22000c100320614ULL,
	0x6408a15011010088ULL, 0x10060c650494c20aULL, 0x98800402088254ULL, 0x1a0200200aa00002ULL,
	0x2221128c0804110ULL, 0x41012002044440a8ULL, 0x748d26000400409ULL, 0x4008c4000520205ULL,
	0x164260220398211ULL, 0x608208001010190cULL, 0x8c42080002001302ULL, 0x1042020000464140ULL,
	0x1020a20484122088ULL, 0x90100090040008d8ULL, 0xc08804202042104ULL, 0x14c008000029005ULL,
};*/
/*const uint64_t rookMagics[64] = {
	0x2080020500400f0ULL, 0x28444000400010ULL, 0x20000a1004100014ULL, 0x20010c090202006ULL,
	0x8408008200810004ULL, 0x1746000808002ULL, 0x2200098000808201ULL, 0x12c0002080200041ULL,
	0x104000208e480804ULL, 0x8084014008281008ULL, 0x4200810910500410ULL, 0x100014481c20400cULL,
	0x4014a4040020808ULL, 0x401002001010a4ULL, 0x202000500010001ULL, 0x8112808005810081ULL,
	0x40902108802020ULL, 0x42002101008101ULL, 0x459442200810c202ULL, 0x81001103309808ULL,
	0x8110000080102ULL, 0x8812806008080404ULL, 0x104020000800101ULL, 0x40a1048000028201ULL,
	0x4100ba0000004081ULL, 0x44803a4003400109ULL, 0xa010a00000030443ULL, 0x91021a000100409ULL,
	0x4201e8040880a012ULL, 0x22a000440201802ULL, 0x30890a72000204ULL, 0x10411402a0c482ULL,
	0x40004841102088ULL, 0x40230000100040ULL, 0x40100010000a0488ULL, 0x1410100200050844ULL,
	0x100090808508411ULL, 0x1410040024001142ULL, 0x8840018001214002ULL, 0x410201000098001ULL,
	0x8400802120088848ULL, 0x2060080000021004ULL, 0x82101002000d0022ULL, 0x1001101001008241ULL,
	0x9040411808040102ULL, 0x600800480009042ULL, 0x1a020000040205ULL, 0x4200404040505199ULL,
	0x2020081040080080ULL, 0x40a3002000544108ULL, 0x4501100800148402ULL, 0x81440280100224ULL,
	0x88008000000804ULL, 0x8084060000002812ULL, 0x1840201000108312ULL, 0x5080202000000141ULL,
	0x1042a180880281ULL, 0x900802900c01040ULL, 0x8205104104120ULL, 0x9004220000440aULL,
	0x8029510200708ULL, 0x8008440100404241ULL, 0x2420001111000bdULL, 0x4000882304000041ULL,
};

const uint64_t bishopMagics[64] = {
	0x100420000431024ULL, 0x280800101073404ULL, 0x42000a00840802ULL, 0xca800c0410c2ULL,
	0x81004290941c20ULL, 0x400200450020250ULL, 0x444a019204022084ULL, 0x88610802202109aULL,
	0x11210a0800086008ULL, 0x400a08c08802801ULL, 0x1301a0500111c808ULL, 0x1280100480180404ULL,
	0x720009020028445ULL, 0x91880a9000010a01ULL, 0x31200940150802b2ULL, 0x5119080c20000602ULL,
	0x242400a002448023ULL, 0x4819006001200008ULL, 0x222c10400020090ULL, 0x302008420409004ULL,
	0x504200070009045ULL, 0x210071240c02046ULL, 0x1182219000022611ULL, 0x400c50000005801ULL,
	0x4004010000113100ULL, 0x2008121604819400ULL, 0xc4a4010000290101ULL, 0x404a000888004802ULL,
	0x8820c004105010ULL, 0x28280100908300ULL, 0x4c013189c0320a80ULL, 0x42008080042080ULL,
	0x90803000c080840ULL, 0x2180001028220ULL, 0x1084002a040036ULL, 0x212009200401ULL,
	0x128110040c84a84ULL, 0x81488020022802ULL, 0x8c0014100181ULL, 0x2222013020082ULL,
	0xa00100002382c03ULL, 0x1000280001005c02ULL, 0x84801010000114cULL, 0x480410048000084ULL,
	0x21204420080020aULL, 0x2020010000424a10ULL, 0x240041021d500141ULL, 0x420844000280214ULL,
	0x29084a280042108ULL, 0x84102a8080a20a49ULL, 0x104204908010212ULL, 0x40a20280081860c1ULL,
	0x3044000200121004ULL, 0x1001008807081122ULL, 0x50066c000210811ULL, 0xe3001240f8a106ULL,
	0x940c0204030020d4ULL, 0x619204000210826aULL, 0x2010438002b00a2ULL, 0x884042004005802ULL,
	0xa90240000006404ULL, 0x500d082244010008ULL, 0x28190d00040014e0ULL, 0x825201600c082444ULL,
};*/
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

// const uint32_t ROOK_SHIFT = 64 - 12;

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

// const uint32_t BISHOP_SHIFT = 64 - 9;

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

struct MagicBBData
{
	BitBoard* attackData;
	BitBoard mask;
	uint64_t magic;
	uint32_t shift;
};

MagicBBData bishopTable[64], rookTable[64];


BitBoard getRay(uint32_t idx, Direction dir)
{
	return rays[idx][dir];
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

void genAttackData()
{
	for (uint32_t square = 0; square < 64; square++)
	{
		BitBoard bb = ONE << square;

		BitBoard tmp = bb;
		BitBoard result = 0;
		while (tmp)
		{
			tmp = shiftNorth(tmp);
			result |= tmp;
		}
		rays[square][Direction::NORTH] = result;

		tmp = bb;
		result = 0;
		while (tmp)
		{
			tmp = shiftSouth(tmp);
			result |= tmp;
		}
		rays[square][Direction::SOUTH] = result;

		tmp = bb;
		result = 0;
		while (tmp)
		{
			tmp = shiftEast(tmp);
			result |= tmp;
		}
		rays[square][Direction::EAST] = result;

		tmp = bb;
		result = 0;
		while (tmp)
		{
			tmp = shiftWest(tmp);
			result |= tmp;
		}
		rays[square][Direction::WEST] = result;

		tmp = bb;
		result = 0;
		while (tmp)
		{
			tmp = shiftNorthEast(tmp);
			result |= tmp;
		}
		rays[square][Direction::NORTH_EAST] = result;

		tmp = bb;
		result = 0;
		while (tmp)
		{
			tmp = shiftNorthWest(tmp);
			result |= tmp;
		}
		rays[square][Direction::NORTH_WEST] = result;

		tmp = bb;
		result = 0;
		while (tmp)
		{
			tmp = shiftSouthEast(tmp);
			result |= tmp;
		}
		rays[square][Direction::SOUTH_EAST] = result;

		tmp = bb;
		result = 0;
		while (tmp)
		{
			tmp = shiftSouthWest(tmp);
			result |= tmp;
		}
		rays[square][Direction::SOUTH_WEST] = result;

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
	}

	constexpr BitBoard EDGE_SQUARES = files[0] | files[7] | ranks[0] | ranks[7];

	BitBoard* currRook = rookAttacks;
	BitBoard* currBishop = bishopAttacks;
	for (uint32_t square = 0; square < 64; square++)
	{
		// printBB(EDGE_SQUARES);
		BitBoard rookMask =
			(getRay(square, Direction::NORTH) & ~ranks[7]) |
			(getRay(square, Direction::SOUTH) & ~ranks[0]) |
			(getRay(square, Direction::EAST) & ~files[7]) |
			(getRay(square, Direction::WEST) & ~files[0]);

		// printBB(rookMask);
		rookTable[square].magic = rookMagics[square];
		rookTable[square].shift = 64 - rookIndexBits[square];
		rookTable[square].mask = rookMask;
		rookTable[square].attackData = currRook;
		for (uint32_t i = 0; i < (1 << rookIndexBits[square]); i++)
		{
			BitBoard blockers = getMaskBlockerIdx(rookMask, i);
			uint32_t idx = (blockers * rookMagics[square]) >> (64 - rookIndexBits[square]);
			rookTable[square].attackData[idx] = getRookAttacksSlow(square, blockers);
			currRook++;
			// rookAttacks[square][idx] = getRookAttacksSlow(square, blockers);
		}

		BitBoard bishopMask = bishopMasks[square] = ~EDGE_SQUARES &
			(getRay(square, Direction::NORTH_EAST) |
			getRay(square, Direction::NORTH_WEST) |
			getRay(square, Direction::SOUTH_EAST) |
			getRay(square, Direction::SOUTH_WEST));

		bishopTable[square].magic = bishopMagics[square];
		bishopTable[square].shift = 64 - bishopIndexBits[square];
		bishopTable[square].mask = bishopMask;
		bishopTable[square].attackData = currBishop;
		for (uint32_t i = 0; i < (1 << bishopIndexBits[square]); i++)
		{
			BitBoard blockers = getMaskBlockerIdx(bishopMask, i);
			uint32_t idx = (blockers * bishopMagics[square]) >> (64 - bishopIndexBits[square]);
			// if (bishopAttacks[square][idx] != 0)
				// throw std::runtime_error("Bishop magics don't work??");
			bishopTable[square].attackData[idx] = getBishopAttacksSlow(square, blockers);
			currBishop++;
			// bishopAttacks[square][idx] = getBishopAttacksSlow(square, blockers);
		}
	}
}

BitBoard getBishopAttacks(uint32_t square, BitBoard blockers)
{
	blockers &= bishopTable[square].mask;
	return bishopTable[square].attackData[(blockers * bishopTable[square].magic) >> bishopTable[square].shift];
	// blockers &= bishopMasks[square];
	// return bishopAttacks[square][(blockers * bishopMagics[square]) >> (64 - bishopIndexBits[square])];
}

BitBoard getRookAttacks(uint32_t square, BitBoard blockers)
{
	blockers &= rookTable[square].mask;
	return rookTable[square].attackData[(blockers * rookTable[square].magic) >> rookTable[square].shift];
	// blockers &= rookMasks[square];
	// std::cout << blockers << std::endl;
	// printBB(blockers);
	// std::cout << ((blockers * rookMagics[square]) >> (64 - rookIndexBits[square])) << std::endl;
	// return rookAttacks[square][(blockers * rookMagics[square]) >> (64 - rookIndexBits[square])];
}

BitBoard getQueenAttacks(uint32_t square, BitBoard blockers)
{
	return getBishopAttacks(square, blockers) | getRookAttacks(square, blockers);
}

BitBoard getDiagonalAttacks(uint32_t square, BitBoard blockers)
{
	return getBishopAttacks(square, blockers) & (getRay(square, Direction::NORTH_EAST) | getRay(square, Direction::SOUTH_WEST));
}

BitBoard getAntidiagonalAttacks(uint32_t square, BitBoard blockers)
{
	return getBishopAttacks(square, blockers) & (getRay(square, Direction::NORTH_WEST) | getRay(square, Direction::SOUTH_EAST));
}

BitBoard getVerticalAttacks(uint32_t square, BitBoard blockers)
{
	return getRookAttacks(square, blockers) & (getRay(square, Direction::NORTH) | getRay(square, Direction::SOUTH));
}

BitBoard getHorizontalAttacks(uint32_t square, BitBoard blockers)
{
	return getRookAttacks(square, blockers) & (getRay(square, Direction::EAST) | getRay(square, Direction::WEST));
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

BitBoard getDiagonalAttacksSlow(uint32_t square, BitBoard blockers)
{
	BitBoard ray = getRay(square, Direction::NORTH_EAST);
	BitBoard attacks = ray;
	if (BitBoard rayBlockers = ray & blockers)
		attacks ^= getRay(getLSB(rayBlockers), Direction::NORTH_EAST);

	ray = getRay(square, Direction::SOUTH_WEST);
	attacks |= ray;
	if (BitBoard rayBlockers = ray & blockers)
		attacks ^= getRay(getMSB(rayBlockers), Direction::SOUTH_WEST);

	return attacks;
}

BitBoard getAntidiagonalAttacksSlow(uint32_t square, BitBoard blockers)
{
	BitBoard ray = getRay(square, Direction::NORTH_WEST);
	BitBoard attacks = ray;
	if (BitBoard rayBlockers = ray & blockers)
		attacks ^= getRay(getLSB(rayBlockers), Direction::NORTH_WEST);

	ray = getRay(square, Direction::SOUTH_EAST);
	attacks |= ray;
	if (BitBoard rayBlockers = ray & blockers)
		attacks ^= getRay(getMSB(rayBlockers), Direction::SOUTH_EAST);

	return attacks;
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

BitBoard getVerticalAttacksSlow(uint32_t square, BitBoard blockers)
{
	BitBoard ray = getRay(square, Direction::NORTH);
	BitBoard attacks = ray;
	if (BitBoard rayBlockers = ray & blockers)
		attacks ^= getRay(getLSB(rayBlockers), Direction::NORTH);

	ray = getRay(square, Direction::SOUTH);
	attacks |= ray;
	if (BitBoard rayBlockers = ray & blockers)
		attacks ^= getRay(getMSB(rayBlockers), Direction::SOUTH);

	return attacks;
}

BitBoard getHorizontalAttacksSlow(uint32_t square, BitBoard blockers)
{
	BitBoard ray = getRay(square, Direction::EAST);
	BitBoard attacks = ray;
	if (BitBoard rayBlockers = ray & blockers)
		attacks ^= getRay(getLSB(rayBlockers), Direction::EAST);

	ray = getRay(square, Direction::WEST);
	attacks |= ray;
	if (BitBoard rayBlockers = ray & blockers)
		attacks ^= getRay(getMSB(rayBlockers), Direction::WEST);

	return attacks;
}

BitBoard getQueenAttacksSlow(uint32_t square, BitBoard blockers)
{
	return getBishopAttacksSlow(square, blockers) | getRookAttacksSlow(square, blockers);
}

BitBoard getKingAttacks(uint32_t square)
{
	return kingAttacks[square];
}

BitBoard getKnightAttacks(uint32_t square)
{
	return knightAttacks[square];
}
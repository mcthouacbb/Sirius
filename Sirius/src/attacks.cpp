#include "attacks.h"
#include "util/enum_array.h"

namespace attacks
{

template<typename T>
using DirectionArray = EnumArray<T, Direction, 8>;

constexpr std::array<u64, 64> rookMagics = {
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

constexpr std::array<u64, 64> bishopMagics = {
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

// clang-format off
constexpr std::array<u32, 64> rookIndexBits = {
    12, 11, 11, 11, 11, 11, 11, 12,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    12, 11, 11, 11, 11, 11, 11, 12
};

constexpr std::array<u32, 64> bishopIndexBits = {
    6, 5, 5, 5, 5, 5, 5, 6,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    6, 5, 5, 5, 5, 5, 5, 6
};

// clang-format on

std::array<DirectionArray<Bitboard>, 64> rays = {};

constexpr std::array<Direction, 8> allDirections = {Direction::NORTH, Direction::SOUTH,
    Direction::EAST, Direction::WEST, Direction::NORTH_EAST, Direction::NORTH_WEST,
    Direction::SOUTH_EAST, Direction::SOUTH_WEST};

AttackData attackData;

Bitboard getRay(Square sq, Direction dir)
{
    return rays[sq.value()][dir];
}

Bitboard pdep(u64 data, Bitboard mask)
{
    Bitboard result = EMPTY_BB;
    while (mask.any())
    {
        Square lsb = mask.poplsb();
        if (data & 1)
            result |= Bitboard::fromSquare(lsb);
        data >>= 1;
    }
    return result;
}

Bitboard shiftDir(Bitboard b, Direction d)
{
    switch (d)
    {
        case Direction::NORTH:
            return b.north();
        case Direction::SOUTH:
            return b.south();
        case Direction::EAST:
            return b.east();
        case Direction::WEST:
            return b.west();
        case Direction::NORTH_EAST:
            return b.northEast();
        case Direction::NORTH_WEST:
            return b.northWest();
        case Direction::SOUTH_EAST:
            return b.southEast();
        case Direction::SOUTH_WEST:
            return b.southWest();
    }
    assert(false && "Invalid direction in shiftDir");
    return EMPTY_BB;
}

Direction oppositeDirection(Direction d)
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
    assert(false && "Invalid direction in oppositeDirection");
    return Direction(-1);
}

void initRays()
{
    for (u32 square = 0; square < 64; square++)
    {
        Bitboard bb = Bitboard::fromSquare(Square(square));
        for (Direction dir : allDirections)
        {
            Bitboard tmp = bb;
            Bitboard result = EMPTY_BB;
            while (tmp.any())
            {
                tmp = shiftDir(tmp, dir);
                result |= tmp;
            }
            rays[square][dir] = result;
        }
    }
}

Bitboard getRookAttacksSlow(Square square, Bitboard blockers)
{
    Bitboard ray = getRay(square, Direction::NORTH);
    Bitboard attacks = ray;
    if (Bitboard rayBlockers = ray & blockers; rayBlockers.any())
        attacks ^= getRay(rayBlockers.lsb(), Direction::NORTH);

    ray = getRay(square, Direction::SOUTH);
    attacks |= ray;
    if (Bitboard rayBlockers = ray & blockers; rayBlockers.any())
        attacks ^= getRay(rayBlockers.msb(), Direction::SOUTH);

    ray = getRay(square, Direction::EAST);
    attacks |= ray;
    if (Bitboard rayBlockers = ray & blockers; rayBlockers.any())
        attacks ^= getRay(rayBlockers.lsb(), Direction::EAST);

    ray = getRay(square, Direction::WEST);
    attacks |= ray;
    if (Bitboard rayBlockers = ray & blockers; rayBlockers.any())
        attacks ^= getRay(rayBlockers.msb(), Direction::WEST);
    return attacks;
}

Bitboard getBishopAttacksSlow(Square square, Bitboard blockers)
{
    Bitboard ray = getRay(square, Direction::NORTH_EAST);
    Bitboard attacks = ray;
    if (Bitboard rayBlockers = ray & blockers; rayBlockers.any())
        attacks ^= getRay(rayBlockers.lsb(), Direction::NORTH_EAST);

    ray = getRay(square, Direction::NORTH_WEST);
    attacks |= ray;
    if (Bitboard rayBlockers = ray & blockers; rayBlockers.any())
        attacks ^= getRay(rayBlockers.lsb(), Direction::NORTH_WEST);

    ray = getRay(square, Direction::SOUTH_EAST);
    attacks |= ray;
    if (Bitboard rayBlockers = ray & blockers; rayBlockers.any())
        attacks ^= getRay(rayBlockers.msb(), Direction::SOUTH_EAST);

    ray = getRay(square, Direction::SOUTH_WEST);
    attacks |= ray;
    if (Bitboard rayBlockers = ray & blockers; rayBlockers.any())
        attacks ^= getRay(rayBlockers.msb(), Direction::SOUTH_WEST);
    return attacks;
}

void initAttackTables()
{
    for (u32 square = 0; square < 64; square++)
    {
        Bitboard bb = Bitboard::fromSquare(Square(square));
        Bitboard king = bb.east() | bb.west();
        Bitboard lr = king | bb;
        king |= lr.north() | lr.south();

        attackData.kingAttacks[square] = king;

        Bitboard knight = bb.north().northEast() | bb.north().northWest() | bb.south().southEast()
            | bb.south().southWest() | bb.east().northEast() | bb.east().southEast()
            | bb.west().northWest() | bb.west().southWest();
        attackData.knightAttacks[square] = knight;

        attackData.pawnAttacks[static_cast<i32>(Color::WHITE)][square] = pawnAttacks<Color::WHITE>(bb);
        attackData.pawnAttacks[static_cast<i32>(Color::BLACK)][square] = pawnAttacks<Color::BLACK>(bb);
    }

    for (u32 src = 0; src < 64; src++)
    {
        for (u32 dst = 0; dst < 64; dst++)
        {
            if (src == dst)
                continue;
            Bitboard dstBB = Bitboard::fromSquare(Square(dst));
            for (Direction dir : allDirections)
            {
                Bitboard srcRay = getRay(Square(src), dir);
                if ((srcRay & dstBB).any())
                {
                    Bitboard dstRay = getRay(Square(dst), oppositeDirection(dir));
                    attackData.inBetweenSquares[src][dst] = srcRay & dstRay;
                    attackData.alignedSquares[src][dst] = srcRay | dstRay;
                }
            }
        }
    }
}

void initMagics()
{
    constexpr Bitboard EDGE_SQUARES = FILE_A_BB | FILE_H_BB | RANK_1_BB | RANK_8_BB;
    Bitboard* currRook = attackData.rookAttacks.data();
    Bitboard* currBishop = attackData.bishopAttacks.data();
    for (u32 square = 0; square < 64; square++)
    {
        Bitboard rookMask = (getRay(Square(square), Direction::NORTH) & ~RANK_8_BB)
            | (getRay(Square(square), Direction::SOUTH) & ~RANK_1_BB)
            | (getRay(Square(square), Direction::EAST) & ~FILE_H_BB)
            | (getRay(Square(square), Direction::WEST) & ~FILE_A_BB);

        attackData.rookTable[square].magic = rookMagics[square];
        attackData.rookTable[square].shift = 64 - rookIndexBits[square];
        attackData.rookTable[square].mask = rookMask;
        attackData.rookTable[square].attackData = currRook;

        for (u32 i = 0; i < (1u << rookIndexBits[square]); i++)
        {
            Bitboard blockers = pdep(i, rookMask);
            u32 idx = static_cast<u32>(
                (blockers.value() * rookMagics[square]) >> (64 - rookIndexBits[square]));
            attackData.rookTable[square].attackData[idx] = getRookAttacksSlow(Square(square), blockers);
            currRook++;
        }

        Bitboard bishopMask = ~EDGE_SQUARES
            & (getRay(Square(square), Direction::NORTH_EAST)
                | getRay(Square(square), Direction::NORTH_WEST)
                | getRay(Square(square), Direction::SOUTH_EAST)
                | getRay(Square(square), Direction::SOUTH_WEST));

        attackData.bishopTable[square].magic = bishopMagics[square];
        attackData.bishopTable[square].shift = 64 - bishopIndexBits[square];
        attackData.bishopTable[square].mask = bishopMask;
        attackData.bishopTable[square].attackData = currBishop;
        for (u32 i = 0; i < (1u << bishopIndexBits[square]); i++)
        {
            Bitboard blockers = pdep(i, bishopMask);
            u32 idx = static_cast<u32>(
                (blockers.value() * bishopMagics[square]) >> (64 - bishopIndexBits[square]));

            attackData.bishopTable[square].attackData[idx] =
                getBishopAttacksSlow(Square(square), blockers);
            currBishop++;
        }
    }
}

void initEvalTables()
{
    for (i32 i = 0; i < 64; i++)
    {
        Bitboard white = Bitboard::fromSquare(Square(i)) << 8;
        white |= white << 8;
        white |= white << 16;
        white |= white << 32;
        attackData.passedPawnMasks[static_cast<i32>(Color::WHITE)][i] =
            white | white.west() | white.east();

        Bitboard black = Bitboard::fromSquare(Square(i)) >> 8;
        black |= black >> 8;
        black |= black >> 16;
        black |= black >> 32;
        attackData.passedPawnMasks[static_cast<i32>(Color::BLACK)][i] =
            black | black.west() | black.east();

        Bitboard file = white | black | Bitboard::fromSquare(Square(i));
        attackData.isolatedPawnMasks[i] = file.west() | file.east();
    }

    Bitboard kingFlank = FILE_A_BB | FILE_B_BB | FILE_C_BB | FILE_D_BB;
    Bitboard whiteRanks = RANK_1_BB | RANK_2_BB | RANK_3_BB | RANK_4_BB | RANK_5_BB;
    Bitboard blackRanks = RANK_8_BB | RANK_7_BB | RANK_6_BB | RANK_5_BB | RANK_4_BB;
    for (i32 i : {FILE_A, FILE_B, FILE_C})
    {
        attackData.kingFlanks[static_cast<i32>(Color::WHITE)][i] = kingFlank & whiteRanks;
        attackData.kingFlanks[static_cast<i32>(Color::BLACK)][i] = kingFlank & blackRanks;
    }

    kingFlank = FILE_C_BB | FILE_D_BB | FILE_E_BB | FILE_F_BB;
    for (i32 i : {FILE_D, FILE_E})
    {
        attackData.kingFlanks[static_cast<i32>(Color::WHITE)][i] = kingFlank & whiteRanks;
        attackData.kingFlanks[static_cast<i32>(Color::BLACK)][i] = kingFlank & blackRanks;
    }

    kingFlank = FILE_E_BB | FILE_F_BB | FILE_G_BB | FILE_H_BB;
    for (i32 i : {FILE_F, FILE_G, FILE_H})
    {
        attackData.kingFlanks[static_cast<i32>(Color::WHITE)][i] = kingFlank & whiteRanks;
        attackData.kingFlanks[static_cast<i32>(Color::BLACK)][i] = kingFlank & blackRanks;
    }
}

void init()
{
    initRays();
    initAttackTables();
    initMagics();
    initEvalTables();
}

}

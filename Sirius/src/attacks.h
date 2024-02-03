#pragma once

#include "defs.h"
#include "bitboard.h"

#include <array>

namespace attacks
{

void init();

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

struct AttackData
{
    struct Magic
    {
        BitBoard* attackData;
        BitBoard mask;
        uint64_t magic;
        uint32_t shift;
    };

    std::array<std::array<BitBoard, 64>, 64> inBetweenSquares;
    std::array<std::array<BitBoard, 64>, 64> pinRays;
    std::array<std::array<BitBoard, 64>, 64> moveMasks;
    std::array<std::array<BitBoard, 64>, 64> alignedSquares;

    std::array<std::array<BitBoard, 64>, 2> pawnAttacks;
    std::array<BitBoard, 64> kingAttacks;
    std::array<BitBoard, 64> knightAttacks;

    std::array<int, 64> castleRightsMasks;

    Magic bishopTable[64];
    Magic rookTable[64];

    std::array<BitBoard, 102400> rookAttacks;
    std::array<BitBoard, 5248> bishopAttacks;
};

extern AttackData attackData;

template<Color c>
inline BitBoard pawnEastAttacks(BitBoard pawns)
{
    if constexpr (c == Color::WHITE)
        return shiftNorthEast(pawns);
    else
        return shiftSouthEast(pawns);
}

template<Color c>
inline BitBoard pawnWestAttacks(BitBoard pawns)
{
    if constexpr (c == Color::WHITE)
        return shiftNorthWest(pawns);
    else
        return shiftSouthWest(pawns);
}

template<Color c>
inline BitBoard pawnAttacks(BitBoard pawns)
{
    return pawnWestAttacks<c>(pawns) | pawnEastAttacks<c>(pawns);
}

template<Color c>
inline BitBoard pawnPushes(BitBoard pawns)
{
    if constexpr (c == Color::WHITE)
        return shiftNorth(pawns);
    else
        return shiftSouth(pawns);
}

template<Color c>
inline constexpr int pawnPushOffset()
{
    return c == Color::WHITE ? 8 : -8;
}

template<Color c>
inline constexpr BitBoard kscBlockSquares()
{
    if constexpr (c == Color::WHITE)
        return 0x60;
    else
        return 0x6000000000000000;
}

template<Color c>
inline constexpr BitBoard qscBlockSquares()
{
    if constexpr (c == Color::WHITE)
        return 0xE;
    else
        return 0xE00000000000000;
}

inline bool aligned(uint32_t a, uint32_t b, uint32_t c)
{
    return attackData.alignedSquares[a][b] & (1ull << c);
}

inline BitBoard inBetweenSquares(uint32_t src, uint32_t dst)
{
    return attackData.inBetweenSquares[src][dst];
}

inline BitBoard pinRay(uint32_t king, uint32_t pinned)
{
    return attackData.pinRays[king][pinned];
}

inline BitBoard moveMask(uint32_t king, uint32_t checker)
{
    return attackData.moveMasks[king][checker];
}

inline int castleRightsMask(uint32_t square)
{
    return attackData.castleRightsMasks[square];
}

inline BitBoard pawnAttacks(Color color, uint32_t square)
{
    return attackData.pawnAttacks[static_cast<int>(color)][square];
}

inline BitBoard kingAttacks(uint32_t square)
{
    return attackData.kingAttacks[square];
}

inline BitBoard knightAttacks(uint32_t square)
{
    return attackData.knightAttacks[square];
}

inline BitBoard bishopAttacks(uint32_t square, BitBoard blockers)
{
    blockers &= attackData.bishopTable[square].mask;
    uint64_t index = blockers * attackData.bishopTable[square].magic;
    return attackData.bishopTable[square].attackData[index >> attackData.bishopTable[square].shift];
}

inline BitBoard rookAttacks(uint32_t square, BitBoard blockers)
{
    blockers &= attackData.rookTable[square].mask;
    uint64_t index = blockers * attackData.rookTable[square].magic;
    return attackData.rookTable[square].attackData[index >> attackData.rookTable[square].shift];
}

inline BitBoard queenAttacks(uint32_t square, BitBoard blockers)
{
    return bishopAttacks(square, blockers) | rookAttacks(square, blockers);
}

template<PieceType pce>
inline BitBoard pieceAttacks(uint32_t square, BitBoard blockers)
{
    static_assert(pce != PieceType::PAWN, "Cannot use pieceAttacks for pawns");
    switch (pce)
    {
        case PieceType::KNIGHT: return knightAttacks(square);
        case PieceType::BISHOP: return bishopAttacks(square, blockers);
        case PieceType::ROOK: return rookAttacks(square, blockers);
        case PieceType::QUEEN: return queenAttacks(square, blockers);
        case PieceType::KING: return kingAttacks(square);
    }
}


}

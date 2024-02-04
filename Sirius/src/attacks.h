#pragma once

#include "defs.h"
#include "bitboard.h"

#include <array>
#include <utility>

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
        Bitboard* attackData;
        Bitboard mask;
        uint64_t magic;
        uint32_t shift;
    };

    std::array<std::array<Bitboard, 64>, 64> inBetweenSquares;
    std::array<std::array<Bitboard, 64>, 64> pinRays;
    std::array<std::array<Bitboard, 64>, 64> moveMasks;
    std::array<std::array<Bitboard, 64>, 64> alignedSquares;

    std::array<std::array<Bitboard, 64>, 2> pawnAttacks;
    std::array<Bitboard, 64> kingAttacks;
    std::array<Bitboard, 64> knightAttacks;

    std::array<int, 64> castleRightsMasks;

    Magic bishopTable[64];
    Magic rookTable[64];

    std::array<Bitboard, 102400> rookAttacks;
    std::array<Bitboard, 5248> bishopAttacks;
};

extern AttackData attackData;

template<Color c>
inline Bitboard pawnEastAttacks(Bitboard pawns)
{
    if constexpr (c == Color::WHITE)
        return pawns.northEast();
    else
        return pawns.southEast();
}

template<Color c>
inline Bitboard pawnWestAttacks(Bitboard pawns)
{
    if constexpr (c == Color::WHITE)
        return pawns.northWest();
    else
        return pawns.southWest();
}

template<Color c>
inline Bitboard pawnAttacks(Bitboard pawns)
{
    return pawnWestAttacks<c>(pawns) | pawnEastAttacks<c>(pawns);
}

template<Color c>
inline Bitboard pawnPushes(Bitboard pawns)
{
    if constexpr (c == Color::WHITE)
        return pawns.north();
    else
        return pawns.south();
}

template<Color c>
inline constexpr int pawnPushOffset()
{
    return c == Color::WHITE ? 8 : -8;
}

template<Color c>
inline constexpr Bitboard kscBlockSquares()
{
    if constexpr (c == Color::WHITE)
        return 0x60;
    else
        return 0x6000000000000000;
}

template<Color c>
inline constexpr Bitboard qscBlockSquares()
{
    if constexpr (c == Color::WHITE)
        return 0xE;
    else
        return 0xE00000000000000;
}

inline bool aligned(uint32_t a, uint32_t b, uint32_t c)
{
    return (attackData.alignedSquares[a][b] & Bitboard::fromSquare(c)).any();
}

inline Bitboard inBetweenSquares(uint32_t src, uint32_t dst)
{
    return attackData.inBetweenSquares[src][dst];
}

inline Bitboard moveMask(uint32_t king, uint32_t checker)
{
    return attackData.moveMasks[king][checker];
}

inline int castleRightsMask(uint32_t square)
{
    return attackData.castleRightsMasks[square];
}

inline Bitboard pawnAttacks(Color color, uint32_t square)
{
    return attackData.pawnAttacks[static_cast<int>(color)][square];
}

inline Bitboard kingAttacks(uint32_t square)
{
    return attackData.kingAttacks[square];
}

inline Bitboard knightAttacks(uint32_t square)
{
    return attackData.knightAttacks[square];
}

inline Bitboard bishopAttacks(uint32_t square, Bitboard blockers)
{
    blockers &= attackData.bishopTable[square].mask;
    uint64_t index = blockers.value() * attackData.bishopTable[square].magic;
    return attackData.bishopTable[square].attackData[index >> attackData.bishopTable[square].shift];
}

inline Bitboard rookAttacks(uint32_t square, Bitboard blockers)
{
    blockers &= attackData.rookTable[square].mask;
    uint64_t index = blockers.value() * attackData.rookTable[square].magic;
    return attackData.rookTable[square].attackData[index >> attackData.rookTable[square].shift];
}

inline Bitboard queenAttacks(uint32_t square, Bitboard blockers)
{
    return bishopAttacks(square, blockers) | rookAttacks(square, blockers);
}

template<PieceType pce>
inline Bitboard pieceAttacks(uint32_t square, Bitboard blockers)
{
    static_assert(
        pce == PieceType::KNIGHT ||
        pce == PieceType::BISHOP ||
        pce == PieceType::ROOK ||
        pce == PieceType::QUEEN ||
        pce == PieceType::KING, "invalid piece type for attacks::pieceAttacks");
    switch (pce)
    {
        case PieceType::KNIGHT: return knightAttacks(square);
        case PieceType::BISHOP: return bishopAttacks(square, blockers);
        case PieceType::ROOK: return rookAttacks(square, blockers);
        case PieceType::QUEEN: return queenAttacks(square, blockers);
        case PieceType::KING: return kingAttacks(square);
        // unreachable
        default: return 0;
    }
}


}

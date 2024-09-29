#pragma once

#include "defs.h"
#include "bitboard.h"
#include "util/multi_array.h"

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

    MultiArray<Bitboard, 64, 64> inBetweenSquares;
    MultiArray<Bitboard, 64, 64> moveMasks;
    MultiArray<Bitboard, 64, 64> alignedSquares;

    MultiArray<Bitboard, 2, 64> passedPawnMasks;
    std::array<Bitboard, 64> isolatedPawnMasks;

    MultiArray<Bitboard, 2, 64> pawnAttacks;
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
inline Bitboard fillUp(Bitboard bb)
{
    if constexpr (c == Color::WHITE)
    {
        bb |= bb << 8;
        bb |= bb << 16;
        bb |= bb << 32;
        return bb;
    }
    else
    {
        bb |= bb >> 8;
        bb |= bb >> 16;
        bb |= bb >> 32;
        return bb;
    }
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
        return Bitboard(0x60);
    else
        return Bitboard(0x6000000000000000);
}

template<Color c>
inline constexpr Bitboard qscBlockSquares()
{
    if constexpr (c == Color::WHITE)
        return Bitboard(0xE);
    else
        return Bitboard(0xE00000000000000);
}

inline bool aligned(Square a, Square b, Square c)
{
    return (attackData.alignedSquares[a.value()][b.value()] & Bitboard::fromSquare(c)).any();
}

inline Bitboard inBetweenSquares(Square src, Square dst)
{
    return attackData.inBetweenSquares[src.value()][dst.value()];
}

inline Bitboard moveMask(Square king, Square checker)
{
    return attackData.moveMasks[king.value()][checker.value()];
}

inline CastlingRights castleRightsMask(Square square)
{
    return CastlingRights(static_cast<CastlingRights::Internal>(attackData.castleRightsMasks[square.value()]));
}

inline Bitboard passedPawnMask(Color color, Square square)
{
    return attackData.passedPawnMasks[static_cast<int>(color)][square.value()];
}

inline Bitboard isolatedPawnMask(Square square)
{
    return attackData.isolatedPawnMasks[square.value()];
}

inline Bitboard pawnAttacks(Color color, Square square)
{
    return attackData.pawnAttacks[static_cast<int>(color)][square.value()];
}

inline Bitboard kingAttacks(Square square)
{
    return attackData.kingAttacks[square.value()];
}

inline Bitboard knightAttacks(Square square)
{
    return attackData.knightAttacks[square.value()];
}

inline Bitboard bishopAttacks(Square square, Bitboard blockers)
{
    blockers &= attackData.bishopTable[square.value()].mask;
    uint64_t index = blockers.value() * attackData.bishopTable[square.value()].magic;
    return attackData.bishopTable[square.value()].attackData[index >> attackData.bishopTable[square.value()].shift];
}

inline Bitboard rookAttacks(Square square, Bitboard blockers)
{
    blockers &= attackData.rookTable[square.value()].mask;
    uint64_t index = blockers.value() * attackData.rookTable[square.value()].magic;
    return attackData.rookTable[square.value()].attackData[index >> attackData.rookTable[square.value()].shift];
}

inline Bitboard queenAttacks(Square square, Bitboard blockers)
{
    return bishopAttacks(square, blockers) | rookAttacks(square, blockers);
}

template<PieceType pce>
inline Bitboard pieceAttacks(Square square, Bitboard blockers)
{
    using enum PieceType;
    static_assert(
        pce == KNIGHT ||
        pce == BISHOP ||
        pce == ROOK ||
        pce == QUEEN ||
        pce == KING, "invalid piece type for attacks::pieceAttacks");
    switch (pce)
    {
        case KNIGHT: return knightAttacks(square);
        case BISHOP: return bishopAttacks(square, blockers);
        case ROOK: return rookAttacks(square, blockers);
        case QUEEN: return queenAttacks(square, blockers);
        case KING: return kingAttacks(square);
        // unreachable
        default: return Bitboard(0);
    }
}


}

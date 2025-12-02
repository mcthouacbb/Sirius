#pragma once

#include "bitboard.h"
#include "defs.h"
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
        u64 magic;
        u32 shift;
    };

    MultiArray<Bitboard, 64, 64> inBetweenSquares;
    MultiArray<Bitboard, 64, 64> alignedSquares;

    MultiArray<Bitboard, 2, 64> passedPawnMasks;
    std::array<Bitboard, 64> isolatedPawnMasks;
    MultiArray<Bitboard, 2, 8> kingFlanks;

    MultiArray<Bitboard, 2, 64> pawnAttacks;
    std::array<Bitboard, 64> kingAttacks;
    std::array<Bitboard, 64> knightAttacks;

    Magic bishopTable[64];
    Magic rookTable[64];

    std::array<Bitboard, 102400> rookAttacks;
    std::array<Bitboard, 5248> bishopAttacks;
};

extern AttackData attackData;

template<Color c>
constexpr Bitboard pawnEastAttacks(Bitboard pawns)
{
    if constexpr (c == Color::WHITE)
        return pawns.northEast();
    else
        return pawns.southEast();
}

template<Color c>
constexpr Bitboard pawnWestAttacks(Bitboard pawns)
{
    if constexpr (c == Color::WHITE)
        return pawns.northWest();
    else
        return pawns.southWest();
}

template<Color c>
constexpr Bitboard pawnAttacks(Bitboard pawns)
{
    return pawnWestAttacks<c>(pawns) | pawnEastAttacks<c>(pawns);
}

template<Color c>
constexpr Bitboard pawnPushes(Bitboard pawns)
{
    if constexpr (c == Color::WHITE)
        return pawns.north();
    else
        return pawns.south();
}

template<Color c>
constexpr Bitboard fillUp(Bitboard bb)
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

constexpr Bitboard fillUp(Bitboard bb, Color c)
{
    if (c == Color::WHITE)
        return attacks::fillUp<Color::WHITE>(bb);
    else
        return attacks::fillUp<Color::BLACK>(bb);
}

template<Color c>
constexpr i32 pawnPushOffset()
{
    return c == Color::WHITE ? 8 : -8;
}

inline Bitboard alignedSquares(Square a, Square b)
{
    return attackData.alignedSquares[a.value()][b.value()];
}

inline bool aligned(Square a, Square b, Square c)
{
    return alignedSquares(a, b).has(c);
}

inline Bitboard inBetweenSquares(Square src, Square dst)
{
    return attackData.inBetweenSquares[src.value()][dst.value()];
}

inline Bitboard moveMask(Square king, Square checker)
{
    return inBetweenSquares(king, checker) | Bitboard::fromSquare(checker);
}

inline Bitboard passedPawnMask(Color color, Square square)
{
    return attackData.passedPawnMasks[static_cast<i32>(color)][square.value()];
}

inline Bitboard isolatedPawnMask(Square square)
{
    return attackData.isolatedPawnMasks[square.value()];
}

inline Bitboard kingFlank(Color color, i32 file)
{
    return attackData.kingFlanks[static_cast<i32>(color)][file];
}

inline Bitboard pawnAttacks(Color color, Square square)
{
    return attackData.pawnAttacks[static_cast<i32>(color)][square.value()];
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
    u64 index = blockers.value() * attackData.bishopTable[square.value()].magic;
    return attackData.bishopTable[square.value()]
        .attackData[index >> attackData.bishopTable[square.value()].shift];
}

inline Bitboard rookAttacks(Square square, Bitboard blockers)
{
    blockers &= attackData.rookTable[square.value()].mask;
    u64 index = blockers.value() * attackData.rookTable[square.value()].magic;
    return attackData.rookTable[square.value()]
        .attackData[index >> attackData.rookTable[square.value()].shift];
}

inline Bitboard queenAttacks(Square square, Bitboard blockers)
{
    return bishopAttacks(square, blockers) | rookAttacks(square, blockers);
}

template<Color color>
constexpr Bitboard kingRing(Square kingSq)
{
    Bitboard kingAtks = attacks::kingAttacks(kingSq);
    Bitboard kingRing = kingAtks | attacks::pawnPushes<color>(kingAtks);
    if (FILE_H_BB.has(kingSq))
        kingRing |= kingRing.west();
    if (FILE_A_BB.has(kingSq))
        kingRing |= kingRing.east();
    return kingRing & ~Bitboard::fromSquare(kingSq);
}

template<PieceType pce>
inline Bitboard pieceAttacks(Square square, Bitboard blockers)
{
    using enum PieceType;
    static_assert(pce == KNIGHT || pce == BISHOP || pce == ROOK || pce == QUEEN || pce == KING,
        "invalid piece type for attacks::pieceAttacks");
    switch (pce)
    {
        case KNIGHT:
            return knightAttacks(square);
        case BISHOP:
            return bishopAttacks(square, blockers);
        case ROOK:
            return rookAttacks(square, blockers);
        case QUEEN:
            return queenAttacks(square, blockers);
        case KING:
            return kingAttacks(square);
        // unreachable
        default:
            return EMPTY_BB;
    }
}

}

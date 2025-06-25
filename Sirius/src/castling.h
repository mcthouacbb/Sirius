#pragma once

#include "attacks.h"
#include "defs.h"
#include "util/enum_array.h"

enum class CastleSide
{
    KING_SIDE,
    QUEEN_SIDE
};

template<typename T>
using CastleSideArray = EnumArray<T, CastleSide, 2>;

struct CastlingRights
{
public:
    enum class Internal : uint8_t
    {
        NONE = 0,
        WHITE_KING_SIDE = 1,
        WHITE_QUEEN_SIDE = 2,
        BLACK_KING_SIDE = 4,
        BLACK_QUEEN_SIDE = 8,
        ALL = 15
    };
    static constexpr Internal NONE = Internal::NONE;
    static constexpr Internal WHITE_KING_SIDE = Internal::WHITE_KING_SIDE;
    static constexpr Internal WHITE_QUEEN_SIDE = Internal::WHITE_QUEEN_SIDE;
    static constexpr Internal BLACK_KING_SIDE = Internal::BLACK_KING_SIDE;
    static constexpr Internal BLACK_QUEEN_SIDE = Internal::BLACK_QUEEN_SIDE;
    static constexpr Internal ALL = Internal::ALL;

    constexpr CastlingRights();
    constexpr CastlingRights(Internal v);
    constexpr CastlingRights(Color color, CastleSide side);

    constexpr CastlingRights& operator&=(const CastlingRights& other);
    constexpr CastlingRights& operator|=(const CastlingRights& other);

    constexpr CastlingRights operator&(const CastlingRights& other) const;
    constexpr CastlingRights operator|(const CastlingRights& other) const;

    constexpr CastlingRights operator~() const;

    constexpr bool has(CastlingRights other) const;
    constexpr bool has(Internal v) const;

    constexpr int value() const;

private:
    Internal m_Value;
};

constexpr CastlingRights operator&(CastlingRights::Internal a, CastlingRights::Internal b)
{
    return CastlingRights(
        static_cast<CastlingRights::Internal>(static_cast<int>(a) & static_cast<int>(b)));
}

constexpr CastlingRights operator|(CastlingRights::Internal a, CastlingRights::Internal b)
{
    return CastlingRights(
        static_cast<CastlingRights::Internal>(static_cast<int>(a) | static_cast<int>(b)));
}

constexpr CastlingRights operator~(CastlingRights::Internal a)
{
    return static_cast<CastlingRights::Internal>(~static_cast<int>(a)) & CastlingRights::ALL;
}

constexpr CastlingRights::CastlingRights()
    : m_Value(Internal::NONE)
{
}

constexpr CastlingRights::CastlingRights(Internal v)
    : m_Value(v)
{
}

constexpr CastlingRights::CastlingRights(Color color, CastleSide side)
    : m_Value(static_cast<Internal>(1 << (2 * static_cast<int>(color) + static_cast<int>(side))))
{
}

constexpr CastlingRights& CastlingRights::operator&=(const CastlingRights& other)
{
    *this = *this & other;
    return *this;
}

constexpr CastlingRights& CastlingRights::operator|=(const CastlingRights& other)
{
    *this = *this | other;
    return *this;
}

constexpr CastlingRights CastlingRights::operator&(const CastlingRights& other) const
{
    return m_Value & other.m_Value;
}

constexpr CastlingRights CastlingRights::operator|(const CastlingRights& other) const
{
    return m_Value | other.m_Value;
}

constexpr CastlingRights CastlingRights::operator~() const
{
    return ~m_Value;
}

constexpr bool CastlingRights::has(CastlingRights other) const
{
    return static_cast<int>((m_Value & other.m_Value).m_Value) != 0;
}

constexpr bool CastlingRights::has(Internal v) const
{
    return static_cast<int>((m_Value & v).m_Value) != 0;
}

constexpr int CastlingRights::value() const
{
    return static_cast<int>(m_Value);
}

struct CastlingData
{
public:
    CastlingData()
    {
        m_RookSquares[Color::WHITE][CastleSide::QUEEN_SIDE] = Square(0, 0);
        m_RookSquares[Color::WHITE][CastleSide::KING_SIDE] = Square(0, 7);
        m_KingSquares[Color::WHITE] = Square(0, 4);

        m_RookSquares[Color::BLACK][CastleSide::QUEEN_SIDE] = Square(7, 0);
        m_RookSquares[Color::BLACK][CastleSide::KING_SIDE] = Square(7, 7);
        m_KingSquares[Color::BLACK] = Square(7, 4);

        m_CastleRightsMasks.fill(CastlingRights::ALL);
        m_BlockSquares = {};
    }

    void setKingSquares(Square whiteKing, Square blackKing)
    {
        m_KingSquares[Color::WHITE] = whiteKing;
        m_KingSquares[Color::BLACK] = blackKing;
    }

    void setRookSquare(Color color, CastleSide side, Square square)
    {
        m_RookSquares[color][side] = square;
    }

    Square rookSquare(Color color, CastleSide side) const
    {
        return m_RookSquares[color][side];
    }

    Bitboard blockSquares(Color color, CastleSide side) const
    {
        return m_BlockSquares[color][side];
    }

    CastlingRights castleRightsMask(Square sq) const
    {
        return m_CastleRightsMasks[sq.value()];
    }

    void initMasks()
    {
        m_CastleRightsMasks.fill(CastlingRights::ALL);
        m_CastleRightsMasks[m_RookSquares[Color::WHITE][CastleSide::KING_SIDE].value()] &=
            ~CastlingRights::WHITE_KING_SIDE;
        m_CastleRightsMasks[m_RookSquares[Color::WHITE][CastleSide::QUEEN_SIDE].value()] &=
            ~CastlingRights::WHITE_QUEEN_SIDE;
        m_CastleRightsMasks[m_KingSquares[Color::WHITE].value()] &=
            ~(CastlingRights::WHITE_KING_SIDE | CastlingRights::WHITE_QUEEN_SIDE);

        m_CastleRightsMasks[m_RookSquares[Color::BLACK][CastleSide::KING_SIDE].value()] &=
            ~CastlingRights::BLACK_KING_SIDE;
        m_CastleRightsMasks[m_RookSquares[Color::BLACK][CastleSide::QUEEN_SIDE].value()] &=
            ~CastlingRights::BLACK_QUEEN_SIDE;
        m_CastleRightsMasks[m_KingSquares[Color::BLACK].value()] &=
            ~(CastlingRights::BLACK_KING_SIDE | CastlingRights::BLACK_QUEEN_SIDE);

        const auto setBlockSquares = [&](Color c, CastleSide s)
        {
            Square kingDst =
                Square(m_KingSquares[c].rank(), s == CastleSide::KING_SIDE ? FILE_G : FILE_C);
            Square rookDst =
                Square(m_RookSquares[c][s].rank(), s == CastleSide::KING_SIDE ? FILE_F : FILE_D);
            m_BlockSquares[c][s] = attacks::inBetweenSquares(m_KingSquares[c], kingDst)
                | attacks::inBetweenSquares(m_RookSquares[c][s], rookDst)
                | Bitboard::fromSquare(kingDst) | Bitboard::fromSquare(rookDst);
            m_BlockSquares[c][s] &=
                ~(Bitboard::fromSquare(m_KingSquares[c]) | Bitboard::fromSquare(m_RookSquares[c][s]));
        };

        setBlockSquares(Color::WHITE, CastleSide::KING_SIDE);
        setBlockSquares(Color::WHITE, CastleSide::QUEEN_SIDE);
        setBlockSquares(Color::BLACK, CastleSide::KING_SIDE);
        setBlockSquares(Color::BLACK, CastleSide::QUEEN_SIDE);
    }

private:
    ColorArray<CastleSideArray<Square>> m_RookSquares;
    ColorArray<Square> m_KingSquares;
    std::array<CastlingRights, 64> m_CastleRightsMasks;
    ColorArray<CastleSideArray<Bitboard>> m_BlockSquares;
};

constexpr Square castleKingDst(Color color, CastleSide side)
{
    if (color == Color::WHITE)
    {
        if (side == CastleSide::KING_SIDE)
            return Square(RANK_1, FILE_G);
        else
            return Square(RANK_1, FILE_C);
    }
    else
    {
        if (side == CastleSide::KING_SIDE)
            return Square(RANK_8, FILE_G);
        else
            return Square(RANK_8, FILE_C);
    }
}

constexpr Square castleRookDst(Color color, CastleSide side)
{
    if (color == Color::WHITE)
    {
        if (side == CastleSide::KING_SIDE)
            return Square(RANK_1, FILE_F);
        else
            return Square(RANK_1, FILE_D);
    }
    else
    {
        if (side == CastleSide::KING_SIDE)
            return Square(RANK_8, FILE_F);
        else
            return Square(RANK_8, FILE_D);
    }
}

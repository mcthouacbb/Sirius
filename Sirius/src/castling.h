#include "defs.h"
#include "util/enum_array.h"
#include "attacks.h"

enum class CastleSide
{
    KING_SIDE,
    QUEEN_SIDE
};

template<typename T>
using CastleSideArray = EnumArray<T, CastleSide, 2>;

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
        m_CastleRightsMasks[m_RookSquares[Color::WHITE][CastleSide::KING_SIDE].value()] &= ~CastlingRights::WHITE_KING_SIDE;
        m_CastleRightsMasks[m_RookSquares[Color::WHITE][CastleSide::QUEEN_SIDE].value()] &= ~CastlingRights::WHITE_QUEEN_SIDE;
        m_CastleRightsMasks[m_KingSquares[Color::WHITE].value()] &= ~(CastlingRights::WHITE_KING_SIDE | CastlingRights::WHITE_QUEEN_SIDE);

        m_CastleRightsMasks[m_RookSquares[Color::BLACK][CastleSide::KING_SIDE].value()] &= ~CastlingRights::BLACK_KING_SIDE;
        m_CastleRightsMasks[m_RookSquares[Color::BLACK][CastleSide::QUEEN_SIDE].value()] &= ~CastlingRights::BLACK_QUEEN_SIDE;
        m_CastleRightsMasks[m_KingSquares[Color::BLACK].value()] &= ~(CastlingRights::BLACK_KING_SIDE | CastlingRights::BLACK_QUEEN_SIDE);

        const auto setBlockSquares = [&](Color c, CastleSide s)
        {
            Square kingDst = Square(m_KingSquares[c].rank(), s == CastleSide::KING_SIDE ? FILE_G : FILE_C);
            Square rookDst = Square(m_RookSquares[c][s].rank(), s == CastleSide::KING_SIDE ? FILE_F : FILE_D);
            m_BlockSquares[c][s] =
                attacks::inBetweenSquares(m_KingSquares[c], kingDst) | Bitboard::fromSquare(kingDst) |
                attacks::inBetweenSquares(m_RookSquares[c][s], rookDst) | Bitboard::fromSquare(rookDst);
            m_BlockSquares[c][s] &= ~(Bitboard::fromSquare(m_KingSquares[c]) | Bitboard::fromSquare(m_RookSquares[c][s]));
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

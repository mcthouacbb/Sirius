#include "defs.h"
#include "util/enum_array.h"

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
    }
private:
    ColorArray<CastleSideArray<Square>> m_RookSquares;
    ColorArray<Square> m_KingSquares;
    std::array<CastlingRights, 64> m_CastleRightsMasks;
};

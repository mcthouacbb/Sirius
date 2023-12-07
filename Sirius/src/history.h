#pragma once

#include "defs.h"
#include "board.h"
#include <span>

struct ExtMove : public Move
{
public:
    static ExtMove from(const Board& board, Move move);

    Piece movingPiece() const;
    Piece capturedPiece() const;
    Piece promotionPiece() const;
private:
    static Piece promotionPiece(Color sideToMove, Promotion promotion);

    ExtMove(Move move, Piece moving, Piece captured, Piece promotion);

    Piece m_Moving, m_Captured, m_Promotion;
};

inline ExtMove ExtMove::from(const Board& board, Move move)
{
    Piece moving = board.getPieceAt(move.srcPos());
    Piece captured =
        move.type() == MoveType::ENPASSANT ?
        makePiece(PieceType::PAWN, flip(board.sideToMove())) :
        board.getPieceAt(move.dstPos());
    Piece promotion = move.type() == MoveType::PROMOTION ? promotionPiece(board.sideToMove(), move.promotion()) : PIECE_NONE;
    return ExtMove(move, moving, captured, promotion);
}

inline Piece ExtMove::movingPiece() const
{
    return m_Moving;
}

inline Piece ExtMove::capturedPiece() const
{
    return m_Captured;
}

inline Piece ExtMove::promotionPiece() const
{
    return m_Promotion;
}

inline ExtMove::ExtMove(Move move, Piece moving, Piece captured, Piece promotion)
    : Move(move.srcPos(), move.dstPos(), move.type(), move.promotion()), m_Moving(moving), m_Captured(captured), m_Promotion(promotion)
{

}

inline Piece ExtMove::promotionPiece(Color sideToMove, Promotion promotion)
{
    return makePiece(static_cast<PieceType>((static_cast<int>(promotion) >> 14) + static_cast<int>(PieceType::KNIGHT)), sideToMove);
}

template<int MAX_VAL>
struct HistoryEntry
{
public:
    HistoryEntry() = default;
    HistoryEntry(int value);

    operator int() const;

    void update(int bonus);
private:
    int m_Value;
};

template<int MAX_VAL>
inline HistoryEntry<MAX_VAL>::HistoryEntry(int value)
{
    m_Value = value;
}

template<int MAX_VAL>
inline HistoryEntry<MAX_VAL>::operator int() const
{
    return m_Value;
}

template<int MAX_VAL>
inline void HistoryEntry<MAX_VAL>::update(int bonus)
{
    m_Value += bonus - m_Value * std::abs(bonus) / MAX_VAL;
}

static constexpr int HISTORY_MAX = 16384;

using MainHist = std::array<std::array<HistoryEntry<HISTORY_MAX>, 4096>, 2>;

int historyBonus(int depth);

class History
{
public:
    History() = default;

    int getQuietStats(ExtMove move) const;

    void clear();
    void updateQuietStats(ExtMove move, int bonus);
private:
    int getMainHist(ExtMove move) const;

    void updateMainHist(ExtMove move, int bonus);

    MainHist m_MainHist;
};

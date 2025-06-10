#pragma once

#include "board.h"
#include "defs.h"
#include "search_params.h"
#include <algorithm>

struct SearchStack;

// all these functions assume that move is a pseudolegal move on the board
inline Piece movingPiece(const Board& board, Move move)
{
    return board.pieceAt(move.fromSq());
}

inline Piece capturedPiece(const Board& board, Move move)
{
    return move.type() == MoveType::ENPASSANT ? makePiece(PieceType::PAWN, ~board.sideToMove())
                                              : board.pieceAt(move.toSq());
}

// pieces are currently stored as type + 8 * color internally
// however, this wastes indices between
// white king = 5
// and black pawn = 8
// this function packs all the piece indices tightly into indices 0-11
inline int packPieceIndices(Piece piece)
{
    // does not work with NONE pieces
    assert(piece != Piece::NONE);
    PieceType type = getPieceType(piece);
    if (getPieceColor(piece) == Color::WHITE)
        return static_cast<int>(type);
    return static_cast<int>(type) + 6;
}

template<int MAX_VAL>
struct HistoryEntry
{
public:
    HistoryEntry() = default;
    HistoryEntry(int16_t value);

    operator int() const;

    void update(int bonus);

private:
    int16_t m_Value;
};

template<int MAX_VAL>
inline HistoryEntry<MAX_VAL>::HistoryEntry(int16_t value)
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

constexpr int CORR_HIST_SCALE = 256;

struct CorrHistValue
{
public:
    CorrHistValue() = default;
    CorrHistValue(int16_t value);

    operator int() const;

    void update(int target, int weight);

private:
    int16_t m_Value;
};

inline CorrHistValue::CorrHistValue(int16_t value)
{
    m_Value = value;
}

inline CorrHistValue::operator int() const
{
    return m_Value;
}

inline void CorrHistValue::update(int target, int weight)
{
    int newValue = (m_Value * (256 - weight) + target * weight) / 256;
    newValue = std::clamp(
        newValue, m_Value - search::maxCorrHistUpdate, m_Value + search::maxCorrHistUpdate);
    m_Value = static_cast<int16_t>(std::clamp(newValue, -search::maxCorrHist, search::maxCorrHist));
}

struct CorrHistEntry
{
public:
    CorrHistEntry() = default;
    CorrHistEntry(int16_t value);

    int value(int staticEval) const;

    void update(int staticEval, int target, int weight);

private:
    CorrHistValue m_Winning;
    CorrHistValue m_Drawish;
    CorrHistValue m_Losing;
};

inline CorrHistEntry::CorrHistEntry(int16_t value)
    : m_Winning(value), m_Drawish(value), m_Losing(value)
{
}

inline int CorrHistEntry::value(int staticEval) const
{
    if (staticEval >= 200)
        return m_Winning;
    else if (staticEval <= -200)
        return m_Losing;
    else
        return m_Drawish;
}

inline void CorrHistEntry::update(int staticEval, int target, int weight)
{
    if (staticEval >= 200)
        m_Winning.update(target, weight);
    else if (staticEval <= -200)
        m_Losing.update(target, weight);
    else
        m_Drawish.update(target, weight);
}

static constexpr int HISTORY_MAX = 16384;

// main history(~29 elo)
using MainHist = MultiArray<HistoryEntry<HISTORY_MAX>, 2, 4096, 2, 2>;
// continuation history(~40 elo)
using CHEntry = MultiArray<HistoryEntry<HISTORY_MAX>, 12, 64>;
using ContHist = MultiArray<CHEntry, 12, 64>;
// capture history(~19 elo)
using CaptHist = MultiArray<HistoryEntry<HISTORY_MAX>, 7, 12, 64, 2, 2>;

// correction history(~104 elo)
constexpr int CORR_HIST_ENTRIES = 16384;
using PawnCorrHist = MultiArray<CorrHistEntry, 2, CORR_HIST_ENTRIES>;
using NonPawnCorrHist = MultiArray<CorrHistEntry, 2, 2, CORR_HIST_ENTRIES>;
using ThreatsCorrHist = MultiArray<CorrHistEntry, 2, CORR_HIST_ENTRIES>;
using MinorPieceCorrHist = MultiArray<CorrHistEntry, 2, CORR_HIST_ENTRIES>;
using MajorPieceCorrHist = MultiArray<CorrHistEntry, 2, CORR_HIST_ENTRIES>;
using ContCorrEntry = MultiArray<CorrHistEntry, 12, 64>;
using ContCorrHist = MultiArray<ContCorrEntry, 12, 64>;

int historyBonus(int depth);
int historyMalus(int depth);

class History
{
public:
    History() = default;

    CHEntry& contHistEntry(const Board& board, Move move)
    {
        return m_ContHist[packPieceIndices(movingPiece(board, move))][move.toSq().value()];
    }

    const CHEntry& contHistEntry(const Board& board, Move move) const
    {
        return m_ContHist[packPieceIndices(movingPiece(board, move))][move.toSq().value()];
    }

    ContCorrEntry& contCorrEntry(const Board& board, Move move)
    {
        if (move == Move::nullmove())
            return m_ContCorrHist[packPieceIndices(makePiece(PieceType::PAWN, board.sideToMove()))]
                                 [move.toSq().value()];
        return m_ContCorrHist[packPieceIndices(movingPiece(board, move))][move.toSq().value()];
    }

    const ContCorrEntry& contCorrEntry(const Board& board, Move move) const
    {
        if (move == Move::nullmove())
            return m_ContCorrHist[packPieceIndices(makePiece(PieceType::PAWN, board.sideToMove()))]
                                 [move.toSq().value()];
        return m_ContCorrHist[packPieceIndices(movingPiece(board, move))][move.toSq().value()];
    }

    int getQuietStats(
        Move move, Bitboard threats, Piece movingPiece, const SearchStack* stack, int ply) const;
    int getNoisyStats(const Board& board, Move move) const;
    int correctStaticEval(const Board& board, int staticEval, const SearchStack* stack, int ply) const;

    void clear();
    void updateQuietStats(const Board& board, Move move, const SearchStack* stack, int ply, int bonus);
    void updateContHist(Move move, Piece movingPiece, const SearchStack* stack, int ply, int bonus);
    void updateNoisyStats(const Board& board, Move move, int bonus);
    void updateCorrHist(const Board& board, int bonus, int depth, const SearchStack* stack, int ply,
        int rawStaticEval);

private:
    int getMainHist(Move move, Bitboard threats, Color color) const;
    int getContHist(Move move, Piece movingPiece, const CHEntry* entry) const;
    int getCaptHist(const Board& board, Move move) const;

    void updateMainHist(const Board& board, Move move, int bonus);
    void updateContHist(Move move, Piece movingPiece, CHEntry* entry, int bonus);
    void updateCaptHist(const Board& board, Move move, int bonus);

    MainHist m_MainHist;
    ContHist m_ContHist;
    CaptHist m_CaptHist;
    PawnCorrHist m_PawnCorrHist;
    NonPawnCorrHist m_NonPawnCorrHist;
    ThreatsCorrHist m_ThreatsCorrHist;
    MinorPieceCorrHist m_MinorPieceCorrHist;
    MajorPieceCorrHist m_MajorPieceCorrHist;
    ContCorrHist m_ContCorrHist;
};

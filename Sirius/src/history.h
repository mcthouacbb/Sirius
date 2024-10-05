#pragma once

#include "defs.h"
#include "board.h"
#include <span>
#include <algorithm>

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
    Piece moving = board.pieceAt(move.fromSq());
    Piece captured =
        move.type() == MoveType::ENPASSANT ?
        makePiece(PieceType::PAWN, ~board.sideToMove()) :
        board.pieceAt(move.toSq());
    Piece promotion = move.type() == MoveType::PROMOTION ? promotionPiece(board.sideToMove(), move.promotion()) : Piece::NONE;
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
    : Move(move.fromSq(), move.toSq(), move.type(), move.promotion()), m_Moving(moving), m_Captured(captured), m_Promotion(promotion)
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

constexpr int CORR_HIST_SCALE = 256;
constexpr int MAX_CORR_HIST = CORR_HIST_SCALE * 32;

struct CorrHistEntry
{
public:
    CorrHistEntry() = default;
    CorrHistEntry(int value);

    operator int() const;

    void update(int target, int weight);
private:
    int m_Value;
};

inline CorrHistEntry::CorrHistEntry(int value)
{
    m_Value = value;
}

inline CorrHistEntry::operator int() const
{
    return m_Value;
}

inline void CorrHistEntry::update(int target, int weight)
{
    m_Value = (m_Value * (256 - weight) + target * weight) / 256;
    m_Value = std::clamp(m_Value, -MAX_CORR_HIST, MAX_CORR_HIST);
}

static constexpr int HISTORY_MAX = 16384;

using MainHist = MultiArray<HistoryEntry<HISTORY_MAX>, 2, 4096, 2, 2>;
using CHEntry = MultiArray<HistoryEntry<HISTORY_MAX>, 16, 64>;
using ContHist = MultiArray<CHEntry, 16, 64>;
using CaptHist = MultiArray<HistoryEntry<HISTORY_MAX>, 7, 16, 64, 2, 2>;

constexpr int PAWN_CORR_HIST_ENTRIES = 16384;
constexpr int MATERIAL_CORR_HIST_ENTRIES = 32768;
constexpr int NON_PAWN_CORR_HIST_ENTRIES = 16384;
constexpr int THREATS_CORR_HIST_ENTRIES = 16384;
constexpr int MINOR_PIECE_CORR_HIST_ENTRIES = 16384;
constexpr int MAJOR_PIECE_CORR_HIST_ENTRIES = 16384;
using PawnCorrHist = MultiArray<CorrHistEntry, 2, PAWN_CORR_HIST_ENTRIES>;
using MaterialCorrHist = MultiArray<CorrHistEntry, 2, MATERIAL_CORR_HIST_ENTRIES>;
using NonPawnCorrHist = MultiArray<CorrHistEntry, 2, 2, NON_PAWN_CORR_HIST_ENTRIES>;
using ThreatsCorrHist = MultiArray<CorrHistEntry, 2, THREATS_CORR_HIST_ENTRIES>;
using MinorPieceCorrHist = MultiArray<CorrHistEntry, 2, MINOR_PIECE_CORR_HIST_ENTRIES>;
using MajorPieceCorrHist = MultiArray<CorrHistEntry, 2, MAJOR_PIECE_CORR_HIST_ENTRIES>;

int historyBonus(int depth);
int historyMalus(int depth);

class History
{
public:
    History() = default;

    CHEntry& contHistEntry(ExtMove move)
    {
        return m_ContHist[static_cast<int>(move.movingPiece())][move.toSq().value()];
    }

    const CHEntry& contHistEntry(ExtMove move) const
    {
        return m_ContHist[static_cast<int>(move.movingPiece())][move.toSq().value()];
    }

    int getQuietStats(Bitboard threats, ExtMove move, std::span<const CHEntry* const> contHistEntries) const;
    int getNoisyStats(Bitboard threats, ExtMove move) const;
    int correctStaticEval(int staticEval, const Board& board) const;

    void clear();
    void updateQuietStats(Bitboard threats, ExtMove move, std::span<CHEntry*> contHistEntries, int bonus);
    void updateNoisyStats(Bitboard threats, ExtMove move, int bonus);
    void updateCorrHist(int bonus, int depth, const Board& board);
private:
    int getMainHist(Bitboard threats, ExtMove move) const;
    int getContHist(const CHEntry* entry, ExtMove move) const;
    int getCaptHist(Bitboard threats, ExtMove move) const;

    void updateMainHist(Bitboard threats, ExtMove move, int bonus);
    void updateContHist(CHEntry* entry, ExtMove move, int bonus);
    void updateCaptHist(Bitboard threats, ExtMove move, int bonus);

    MainHist m_MainHist;
    ContHist m_ContHist;
    CaptHist m_CaptHist;
    PawnCorrHist m_PawnCorrHist;
    MaterialCorrHist m_MaterialCorrHist;
    NonPawnCorrHist m_NonPawnCorrHist;
    ThreatsCorrHist m_ThreatsCorrHist;
    MinorPieceCorrHist m_MinorPieceCorrHist;
    MajorPieceCorrHist m_MajorPieceCorrHist;
};

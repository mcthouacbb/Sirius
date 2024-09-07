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

template<int MAX_VAL>
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

template<int MAX_VAL>
inline CorrHistEntry<MAX_VAL>::CorrHistEntry(int value)
{
    m_Value = value;
}

template<int MAX_VAL>
inline CorrHistEntry<MAX_VAL>::operator int() const
{
    return m_Value;
}

template<int MAX_VAL>
inline void CorrHistEntry<MAX_VAL>::update(int target, int weight)
{
    m_Value = (m_Value * (256 - weight) + target * weight) / 256;
    m_Value = std::clamp(m_Value, -MAX_VAL, MAX_VAL);
}

static constexpr int HISTORY_MAX = 16384;

using MainHist = std::array<std::array<std::array<std::array<HistoryEntry<HISTORY_MAX>, 2>, 2>, 4096>, 2>;
using CHEntry = std::array<std::array<HistoryEntry<HISTORY_MAX>, 64>, 16>;
using ContHist = std::array<std::array<CHEntry, 64>, 16>;
using CaptHist = std::array<std::array<std::array<HistoryEntry<HISTORY_MAX>, 64>, 16>, 16>;

constexpr int CORR_HIST_SCALE = 256;
constexpr int MAX_CORR_HIST = CORR_HIST_SCALE * 32;
constexpr int PAWN_CORR_HIST_ENTRIES = 16384;
constexpr int MATERIAL_CORR_HIST_ENTRIES = 32768;
constexpr int NON_PAWN_CORR_HIST_ENTRIES = 16384;
constexpr int THREATS_CORR_HIST_ENTRIES = 16384;
constexpr int MINOR_PIECE_CORR_HIST_ENTRIES = 16384;
constexpr int MAJOR_PIECE_CORR_HIST_ENTRIES = 16384;
using PawnCorrHist = std::array<std::array<CorrHistEntry<MAX_CORR_HIST>, PAWN_CORR_HIST_ENTRIES>, 2>;
using MaterialCorrHist = std::array<std::array<CorrHistEntry<MAX_CORR_HIST>, MATERIAL_CORR_HIST_ENTRIES>, 2>;
using NonPawnCorrHist = std::array<std::array<std::array<CorrHistEntry<MAX_CORR_HIST>, NON_PAWN_CORR_HIST_ENTRIES>, 2>, 2>;
using ThreatsCorrHist = std::array<std::array<CorrHistEntry<MAX_CORR_HIST>, THREATS_CORR_HIST_ENTRIES>, 2>;
using MinorPieceCorrHist = std::array<std::array<CorrHistEntry<MAX_CORR_HIST>, MINOR_PIECE_CORR_HIST_ENTRIES>, 2>;
using MajorPieceCorrHist = std::array<std::array<CorrHistEntry<MAX_CORR_HIST>, MAJOR_PIECE_CORR_HIST_ENTRIES>, 2>;

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
    int getNoisyStats(ExtMove move) const;
    int correctStaticEval(int staticEval, const Board& board) const;

    void clear();
    void updateQuietStats(Bitboard threats, ExtMove move, std::span<CHEntry*> contHistEntries, int bonus);
    void updateNoisyStats(ExtMove move, int bonus);
    void updateCorrHist(int bonus, int depth, const Board& board);
private:
    int getMainHist(Bitboard threats, ExtMove move) const;
    int getContHist(const CHEntry* entry, ExtMove move) const;
    int getCaptHist(ExtMove move) const;

    void updateMainHist(Bitboard threats, ExtMove move, int bonus);
    void updateContHist(CHEntry* entry, ExtMove move, int bonus);
    void updateCaptHist(ExtMove move, int bonus);

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

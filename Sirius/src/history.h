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
inline i32 packPieceIndices(Piece piece)
{
    // does not work with NONE pieces
    assert(piece != Piece::NONE);
    PieceType type = getPieceType(piece);
    if (getPieceColor(piece) == Color::WHITE)
        return static_cast<i32>(type);
    return static_cast<i32>(type) + 6;
}

template<i32 MAX_VAL>
struct HistoryEntry
{
public:
    HistoryEntry() = default;
    HistoryEntry(i16 value);

    operator i32() const;

    void update(i32 bonus);
    void update(i32 base, i32 bonus);

private:
    i16 m_Value;
};

template<i32 MAX_VAL>
inline HistoryEntry<MAX_VAL>::HistoryEntry(i16 value)
{
    m_Value = value;
}

template<i32 MAX_VAL>
inline HistoryEntry<MAX_VAL>::operator i32() const
{
    return m_Value;
}

template<i32 MAX_VAL>
inline void HistoryEntry<MAX_VAL>::update(i32 bonus)
{
    update(bonus, m_Value);
}

template<i32 MAX_VAL>
inline void HistoryEntry<MAX_VAL>::update(i32 bonus, i32 base)
{
    i32 newValue = m_Value + bonus - base * std::abs(bonus) / MAX_VAL;
    m_Value = std::clamp(newValue, -MAX_VAL, MAX_VAL);
}

constexpr i32 CORR_HIST_SCALE = 256;

struct CorrHistEntry
{
public:
    CorrHistEntry() = default;
    CorrHistEntry(i16 value);

    operator i32() const;

    void update(i32 target, i32 weight);

private:
    i16 m_Value;
};

inline CorrHistEntry::CorrHistEntry(i16 value)
{
    m_Value = value;
}

inline CorrHistEntry::operator i32() const
{
    return m_Value;
}

inline void CorrHistEntry::update(i32 target, i32 weight)
{
    i32 newValue = (m_Value * (256 - weight) + target * weight) / 256;
    newValue = std::clamp(
        newValue, m_Value - search::maxCorrHistUpdate, m_Value + search::maxCorrHistUpdate);
    m_Value = static_cast<i16>(std::clamp(newValue, -search::maxCorrHist, search::maxCorrHist));
}

static constexpr i32 HISTORY_MAX = 16384;

// main history(~29 elo)
using MainHist = MultiArray<HistoryEntry<HISTORY_MAX>, 2, 4096, 2, 2>;
constexpr i32 PAWN_HIST_ENTRIES = 512;
using PawnHist = MultiArray<HistoryEntry<HISTORY_MAX>, PAWN_HIST_ENTRIES, 12, 64>;
// continuation history(~40 elo)
using CHEntry = MultiArray<HistoryEntry<HISTORY_MAX>, 12, 64>;
using ContHist = MultiArray<CHEntry, 12, 64>;
// capture history(~19 elo)
using CaptHist = MultiArray<HistoryEntry<HISTORY_MAX>, 7, 12, 64, 2, 2>;

constexpr i32 CORR_HIST_ENTRIES = 16384;

struct CorrHist
{
    CorrHistEntry& get(Color color, u64 key)
    {
        return data[static_cast<i32>(color)][key % CORR_HIST_ENTRIES];
    }

    const CorrHistEntry& get(Color color, u64 key) const
    {
        return data[static_cast<i32>(color)][key % CORR_HIST_ENTRIES];
    }

    MultiArray<CorrHistEntry, 2, CORR_HIST_ENTRIES> data;
};

// correction history(~104 elo)
using PawnCorrHist = CorrHist;
using NonPawnCorrHist = ColorArray<CorrHist>;
using ThreatsCorrHist = CorrHist;
using MinorPieceCorrHist = CorrHist;
using MajorPieceCorrHist = CorrHist;
using ContCorrEntry = MultiArray<CorrHistEntry, 12, 64>;
using ContCorrHist = MultiArray<ContCorrEntry, 12, 64>;

i32 historyBonus(i32 depth);
i32 historyMalus(i32 depth);

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

    i32 getQuietStats(Move move, Bitboard threats, Piece movingPiece, ZKey pawnKey,
        const SearchStack* stack, i32 ply) const;
    i32 getNoisyStats(const Board& board, Move move) const;
    i32 correctStaticEval(const Board& board, i32 staticEval, const SearchStack* stack, i32 ply) const;

    void clear();
    void updateQuietStats(const Board& board, Move move, const SearchStack* stack, i32 ply, i32 bonus);
    void updateContHist(Move move, Bitboard threats, Piece movingPiece, const SearchStack* stack, i32 ply, i32 bonus);
    void updateNoisyStats(const Board& board, Move move, i32 bonus);
    void updateCorrHist(const Board& board, i32 bonus, i32 depth, const SearchStack* stack, i32 ply);

private:
    i32 getMainHist(Move move, Bitboard threats, Color color) const;
    i32 getPawnHist(Move move, Piece movingPiece, ZKey pawnKey) const;
    i32 getContHist(Move move, Piece movingPiece, const CHEntry* entry) const;
    i32 getCaptHist(const Board& board, Move move) const;

    void updateMainHist(const Board& board, Move move, i32 bonus);
    void updatePawnHist(const Board& board, Move move, i32 bonus);
    void updateContHist(Move move, Piece movingPiece, CHEntry* entry, i32 base, i32 bonus);
    void updateCaptHist(const Board& board, Move move, i32 bonus);

    MainHist m_MainHist;
    PawnHist m_PawnHist;
    ContHist m_ContHist;
    CaptHist m_CaptHist;
    PawnCorrHist m_PawnCorrHist;
    NonPawnCorrHist m_NonPawnCorrHist;
    ThreatsCorrHist m_ThreatsCorrHist;
    MinorPieceCorrHist m_MinorPieceCorrHist;
    MajorPieceCorrHist m_MajorPieceCorrHist;
    ContCorrHist m_ContCorrHist;
};

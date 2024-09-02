#include "history.h"
#include "search_params.h"
#include <algorithm>

namespace
{

template<int MAX_VAL, size_t N>
void fillHistTable(std::array<HistoryEntry<MAX_VAL>, N>& arr, int value)
{
    std::fill(arr.begin(), arr.end(), value);
}

template<size_t N>
void fillHistTable(std::array<int, N>& arr, int value)
{
    std::fill(arr.begin(), arr.end(), value);
}

template<typename T, size_t N>
void fillHistTable(std::array<T, N>& arr, int value)
{
    for (auto& elem : arr)
    {
        fillHistTable(elem, value);
    }
}

}

int historyBonus(int depth)
{
    // formula from berserk
    return std::min(search::maxHistBonus, search::histBonusQuadratic * depth * depth + search::histBonusLinear * depth - search::histBonusOffset);
}

int historyMalus(int depth)
{
    // formula from berserk
    return std::min(search::maxHistMalus, search::histMalusQuadratic * depth * depth + search::histMalusLinear * depth - search::histMalusOffset);
}


void History::clear()
{
    fillHistTable(m_MainHist, 0);
    fillHistTable(m_PawnHist, 0);
    fillHistTable(m_ContHist, 0);
    fillHistTable(m_CaptHist, 0);
    fillHistTable(m_PawnCorrHist, 0);
    fillHistTable(m_MaterialCorrHist, 0);
    fillHistTable(m_NonPawnCorrHist, 0);
}

int History::getQuietStats(const Board& board, ExtMove move, std::span<const CHEntry* const> contHistEntries) const
{
    int score = getMainHist(board.threats(), move);
    score += getPawnHist(board.pawnKey(), move);
    for (auto entry : contHistEntries)
        if (entry)
            score += getContHist(entry, move);
    return score;
}

int History::getNoisyStats(ExtMove move) const
{
    return getCaptHist(move);
}

int History::correctStaticEval(int staticEval, const Board& board) const
{
    Color stm = board.sideToMove();
    int pawnEntry = m_PawnCorrHist[static_cast<int>(stm)][board.pawnKey().value % PAWN_CORR_HIST_ENTRIES];
    int materialEntry = m_MaterialCorrHist[static_cast<int>(stm)][board.materialKey() % MATERIAL_CORR_HIST_ENTRIES];
    int nonPawnWhiteEntry = m_NonPawnCorrHist[static_cast<int>(stm)][static_cast<int>(Color::WHITE)][board.nonPawnKey(Color::WHITE).value % NON_PAWN_CORR_HIST_ENTRIES];
    int nonPawnBlackEntry = m_NonPawnCorrHist[static_cast<int>(stm)][static_cast<int>(Color::BLACK)][board.nonPawnKey(Color::BLACK).value % NON_PAWN_CORR_HIST_ENTRIES];
    int nonPawnEntry = (nonPawnWhiteEntry + nonPawnBlackEntry) / 2;
    int corrected = staticEval + (pawnEntry + materialEntry + nonPawnEntry) / CORR_HIST_SCALE;
    return std::clamp(corrected, -SCORE_MATE_IN_MAX, SCORE_MATE_IN_MAX);
}

void History::updateQuietStats(const Board& board, ExtMove move, std::span<CHEntry*> contHistEntries, int bonus)
{
    updateMainHist(board.threats(), move, bonus);
    updatePawnHist(board.pawnKey(), move, bonus);
    for (auto entry : contHistEntries)
        if (entry)
            updateContHist(entry, move, bonus);
}

void History::updateNoisyStats(ExtMove move, int bonus)
{
    updateCaptHist(move, bonus);
}

void History::updateCorrHist(int bonus, int depth, const Board& board)
{
    Color stm = board.sideToMove();
    int scaledBonus = bonus * CORR_HIST_SCALE;
    int weight = std::min(1 + depth, 16);

    auto& pawnEntry = m_PawnCorrHist[static_cast<int>(stm)][board.pawnKey().value % PAWN_CORR_HIST_ENTRIES];
    pawnEntry = (pawnEntry * (256 - weight) + scaledBonus * weight) / 256;
    pawnEntry = std::clamp(pawnEntry, -MAX_CORR_HIST, MAX_CORR_HIST);

    auto& materialEntry = m_MaterialCorrHist[static_cast<int>(stm)][board.materialKey() % MATERIAL_CORR_HIST_ENTRIES];
    materialEntry = (materialEntry * (256 - weight) + scaledBonus * weight) / 256;
    materialEntry = std::clamp(materialEntry, -MAX_CORR_HIST, MAX_CORR_HIST);

    auto& nonPawnWhiteEntry = m_NonPawnCorrHist[static_cast<int>(stm)][static_cast<int>(Color::WHITE)][board.nonPawnKey(Color::WHITE).value % NON_PAWN_CORR_HIST_ENTRIES];
    nonPawnWhiteEntry = (nonPawnWhiteEntry * (256 - weight) + scaledBonus * weight) / 256;
    nonPawnWhiteEntry = std::clamp(nonPawnWhiteEntry, -MAX_CORR_HIST, MAX_CORR_HIST);

    auto& nonPawnBlackEntry = m_NonPawnCorrHist[static_cast<int>(stm)][static_cast<int>(Color::BLACK)][board.nonPawnKey(Color::BLACK).value % NON_PAWN_CORR_HIST_ENTRIES];
    nonPawnBlackEntry = (nonPawnBlackEntry * (256 - weight) + scaledBonus * weight) / 256;
    nonPawnBlackEntry = std::clamp(nonPawnBlackEntry, -MAX_CORR_HIST, MAX_CORR_HIST);
}

int History::getMainHist(Bitboard threats, ExtMove move) const
{
    bool srcThreat = (threats & Bitboard::fromSquare(move.fromSq())).any();
    bool dstThreat = (threats & Bitboard::fromSquare(move.toSq())).any();
    return m_MainHist[static_cast<int>(getPieceColor(move.movingPiece()))][move.fromTo()][srcThreat][dstThreat];
}

int History::getPawnHist(ZKey pawnKey, ExtMove move) const
{
    Color stm = getPieceColor(move.movingPiece());
    PieceType piece = getPieceType(move.movingPiece());
    return m_PawnHist[static_cast<int>(stm)][pawnKey.value % PAWN_HIST_ENTRIES][static_cast<int>(piece)][move.toSq().value()];
}

int History::getContHist(const CHEntry* entry, ExtMove move) const
{
    return (*entry)[static_cast<int>(move.movingPiece())][move.toSq().value()];
}

int History::getCaptHist(ExtMove move) const
{
    return m_CaptHist[static_cast<int>(move.capturedPiece())][static_cast<int>(move.movingPiece())][move.toSq().value()];
}

void History::updateMainHist(Bitboard threats, ExtMove move, int bonus)
{
    bool srcThreat = (threats & Bitboard::fromSquare(move.fromSq())).any();
    bool dstThreat = (threats & Bitboard::fromSquare(move.toSq())).any();
    m_MainHist[static_cast<int>(getPieceColor(move.movingPiece()))][move.fromTo()][srcThreat][dstThreat].update(bonus);
}

void History::updatePawnHist(ZKey pawnKey, ExtMove move, int bonus)
{
    Color stm = getPieceColor(move.movingPiece());
    PieceType piece = getPieceType(move.movingPiece());
    m_PawnHist[static_cast<int>(stm)][pawnKey.value % PAWN_HIST_ENTRIES][static_cast<int>(piece)][move.toSq().value()].update(bonus);
}

void History::updateContHist(CHEntry* entry, ExtMove move, int bonus)
{
    (*entry)[static_cast<int>(move.movingPiece())][move.toSq().value()].update(bonus);
}

void History::updateCaptHist(ExtMove move, int bonus)
{
    m_CaptHist[static_cast<int>(move.capturedPiece())][static_cast<int>(move.movingPiece())][move.toSq().value()].update(bonus);
}

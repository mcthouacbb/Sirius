#include "history.h"
#include "search_params.h"
#include "zobrist.h"

namespace
{

template<int MAX_VAL, size_t N>
void fillHistTable(std::array<HistoryEntry<MAX_VAL>, N>& arr, int value)
{
    std::fill(arr.begin(), arr.end(), value);
}

template<size_t N>
void fillHistTable(std::array<CorrHistEntry, N>& arr, int value)
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
        fillHistTable(elem, value);
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
    fillHistTable(m_ContHist, 0);
    fillHistTable(m_CaptHist, 0);
    fillHistTable(m_PawnCorrHist, 0);
    fillHistTable(m_MaterialCorrHist, 0);
    fillHistTable(m_NonPawnCorrHist, 0);
    fillHistTable(m_ThreatsCorrHist, 0);
    fillHistTable(m_MinorPieceCorrHist, 0);
    fillHistTable(m_MajorPieceCorrHist, 0);
}

int History::getQuietStats(Bitboard threats, ExtMove move, std::span<const CHEntry* const> contHistEntries) const
{
    int score = getMainHist(threats, move);
    for (auto entry : contHistEntries)
        if (entry)
            score += getContHist(entry, move);
    return score;
}

int History::getNoisyStats(Bitboard threats, ExtMove move) const
{
    return getCaptHist(threats, move);
}

int History::correctStaticEval(int staticEval, const Board& board) const
{
    Color stm = board.sideToMove();
    uint64_t threatsKey = murmurHash3((board.threats() & board.pieces(stm)).value());
    int pawnEntry = m_PawnCorrHist[static_cast<int>(stm)][board.pawnKey().value % PAWN_CORR_HIST_ENTRIES];
    int materialEntry = m_MaterialCorrHist[static_cast<int>(stm)][board.materialKey() % MATERIAL_CORR_HIST_ENTRIES];
    int nonPawnWhiteEntry = m_NonPawnCorrHist[static_cast<int>(stm)][static_cast<int>(Color::WHITE)][board.nonPawnKey(Color::WHITE).value % NON_PAWN_CORR_HIST_ENTRIES];
    int nonPawnBlackEntry = m_NonPawnCorrHist[static_cast<int>(stm)][static_cast<int>(Color::BLACK)][board.nonPawnKey(Color::BLACK).value % NON_PAWN_CORR_HIST_ENTRIES];
    int nonPawnEntry = (nonPawnWhiteEntry + nonPawnBlackEntry) / 2;
    int threatsEntry = m_ThreatsCorrHist[static_cast<int>(stm)][threatsKey % THREATS_CORR_HIST_ENTRIES];
    int minorPieceEntry = m_MinorPieceCorrHist[static_cast<int>(stm)][board.minorPieceKey().value % MINOR_PIECE_CORR_HIST_ENTRIES];
    int majorPieceEntry = m_MajorPieceCorrHist[static_cast<int>(stm)][board.majorPieceKey().value % MAJOR_PIECE_CORR_HIST_ENTRIES];

    int corrected = staticEval + (pawnEntry + materialEntry + nonPawnEntry + threatsEntry + minorPieceEntry + majorPieceEntry) / CORR_HIST_SCALE;
    return std::clamp(corrected, -SCORE_MATE_IN_MAX, SCORE_MATE_IN_MAX);
}

void History::updateQuietStats(Bitboard threats, ExtMove move, std::span<CHEntry*> contHistEntries, int bonus)
{
    updateMainHist(threats, move, bonus);
    for (auto entry : contHistEntries)
        if (entry)
            updateContHist(entry, move, bonus);
}

void History::updateNoisyStats(Bitboard threats, ExtMove move, int bonus)
{
    updateCaptHist(threats, move, bonus);
}

void History::updateCorrHist(int bonus, int depth, const Board& board)
{
    Color stm = board.sideToMove();
    uint64_t threatsKey = murmurHash3((board.threats() & board.pieces(stm)).value());
    int scaledBonus = bonus * CORR_HIST_SCALE;
    int weight = std::min(1 + depth, 16);

    auto& pawnEntry = m_PawnCorrHist[static_cast<int>(stm)][board.pawnKey().value % PAWN_CORR_HIST_ENTRIES];
    pawnEntry.update(scaledBonus, weight);

    auto& materialEntry = m_MaterialCorrHist[static_cast<int>(stm)][board.materialKey() % MATERIAL_CORR_HIST_ENTRIES];
    materialEntry.update(scaledBonus, weight);

    auto& nonPawnWhiteEntry = m_NonPawnCorrHist[static_cast<int>(stm)][static_cast<int>(Color::WHITE)][board.nonPawnKey(Color::WHITE).value % NON_PAWN_CORR_HIST_ENTRIES];
    nonPawnWhiteEntry.update(scaledBonus, weight);

    auto& nonPawnBlackEntry = m_NonPawnCorrHist[static_cast<int>(stm)][static_cast<int>(Color::BLACK)][board.nonPawnKey(Color::BLACK).value % NON_PAWN_CORR_HIST_ENTRIES];
    nonPawnBlackEntry.update(scaledBonus, weight);

    auto& threatsEntry = m_ThreatsCorrHist[static_cast<int>(stm)][threatsKey % THREATS_CORR_HIST_ENTRIES];
    threatsEntry.update(scaledBonus, weight);

    auto& minorPieceEntry = m_MinorPieceCorrHist[static_cast<int>(stm)][board.minorPieceKey().value % MINOR_PIECE_CORR_HIST_ENTRIES];
    minorPieceEntry.update(scaledBonus, weight);

    auto& majorPieceEntry = m_MajorPieceCorrHist[static_cast<int>(stm)][board.majorPieceKey().value % MAJOR_PIECE_CORR_HIST_ENTRIES];
    majorPieceEntry.update(scaledBonus, weight);
}

int History::getMainHist(Bitboard threats, ExtMove move) const
{
    bool srcThreat = (threats & Bitboard::fromSquare(move.fromSq())).any();
    bool dstThreat = (threats & Bitboard::fromSquare(move.toSq())).any();
    return m_MainHist[static_cast<int>(getPieceColor(move.movingPiece()))][move.fromTo()][srcThreat][dstThreat];
}

int History::getContHist(const CHEntry* entry, ExtMove move) const
{
    return (*entry)[static_cast<int>(move.movingPiece())][move.toSq().value()];
}

int History::getCaptHist(Bitboard threats, ExtMove move) const
{
    bool srcThreat = (threats & Bitboard::fromSquare(move.fromSq())).any();
    bool dstThreat = (threats & Bitboard::fromSquare(move.toSq())).any();
    return m_CaptHist[static_cast<int>(getPieceType(move.capturedPiece()))][static_cast<int>(move.movingPiece())][move.toSq().value()][srcThreat][dstThreat];
}

void History::updateMainHist(Bitboard threats, ExtMove move, int bonus)
{
    bool srcThreat = (threats & Bitboard::fromSquare(move.fromSq())).any();
    bool dstThreat = (threats & Bitboard::fromSquare(move.toSq())).any();
    m_MainHist[static_cast<int>(getPieceColor(move.movingPiece()))][move.fromTo()][srcThreat][dstThreat].update(bonus);
}

void History::updateContHist(CHEntry* entry, ExtMove move, int bonus)
{
    (*entry)[static_cast<int>(move.movingPiece())][move.toSq().value()].update(bonus);
}

void History::updateCaptHist(Bitboard threats, ExtMove move, int bonus)
{
    bool srcThreat = (threats & Bitboard::fromSquare(move.fromSq())).any();
    bool dstThreat = (threats & Bitboard::fromSquare(move.toSq())).any();
    m_CaptHist[static_cast<int>(getPieceType(move.capturedPiece()))][static_cast<int>(move.movingPiece())][move.toSq().value()][srcThreat][dstThreat].update(bonus);
}

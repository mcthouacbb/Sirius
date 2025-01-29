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
    fillHistTable(m_NonPawnCorrHist, 0);
    fillHistTable(m_ThreatsCorrHist, 0);
    fillHistTable(m_MinorPieceCorrHist, 0);
    fillHistTable(m_MajorPieceCorrHist, 0);
}

int History::getQuietStats(Move move, Bitboard threats, Piece movingPiece, std::span<const CHEntry* const> contHistEntries) const
{
    int score = getMainHist(move, threats, getPieceColor(movingPiece));
    for (auto entry : contHistEntries)
        if (entry)
            score += getContHist(move, movingPiece, entry);
    return score;
}

int History::getNoisyStats(const Board& board, Move move) const
{
    return getCaptHist(board, move);
}

int History::correctStaticEval(const Board& board, int staticEval, Move prevMove, Piece prevPiece, const ContCorrEntry* contCorr2) const
{
    Color stm = board.sideToMove();
    uint64_t threatsKey = murmurHash3((board.threats() & board.pieces(stm)).value());
    int pawnEntry = m_PawnCorrHist[static_cast<int>(stm)][board.pawnKey().value % PAWN_CORR_HIST_ENTRIES];
    int nonPawnStmEntry = m_NonPawnCorrHist[static_cast<int>(stm)][static_cast<int>(stm)][board.nonPawnKey(stm).value % NON_PAWN_CORR_HIST_ENTRIES];
    int nonPawnNstmEntry = m_NonPawnCorrHist[static_cast<int>(stm)][static_cast<int>(~stm)][board.nonPawnKey(~stm).value % NON_PAWN_CORR_HIST_ENTRIES];
    int threatsEntry = m_ThreatsCorrHist[static_cast<int>(stm)][threatsKey % THREATS_CORR_HIST_ENTRIES];
    int minorPieceEntry = m_MinorPieceCorrHist[static_cast<int>(stm)][board.minorPieceKey().value % MINOR_PIECE_CORR_HIST_ENTRIES];
    int majorPieceEntry = m_MajorPieceCorrHist[static_cast<int>(stm)][board.majorPieceKey().value % MAJOR_PIECE_CORR_HIST_ENTRIES];

    int correction = 0;
    correction += search::pawnCorrWeight * pawnEntry;
    correction += search::nonPawnStmCorrWeight * nonPawnStmEntry;
    correction += search::nonPawnNstmCorrWeight * nonPawnNstmEntry;
    correction += search::threatsCorrWeight * threatsEntry;
    correction += search::minorCorrWeight * minorPieceEntry;
    correction += search::majorCorrWeight * majorPieceEntry;

    if (prevPiece == Piece::NONE)
        prevPiece = makePiece(PieceType::PAWN, ~board.sideToMove());
    if (contCorr2)
        correction += search::contCorr2Weight * (*contCorr2)[packPieceIndices(prevPiece)][prevMove.toSq().value()];;

    int corrected = staticEval + correction / (256 * CORR_HIST_SCALE);
    return std::clamp(corrected, -SCORE_MATE_IN_MAX + 1, SCORE_MATE_IN_MAX - 1);
}

void History::updateQuietStats(const Board& board, Move move, std::span<CHEntry*> contHistEntries, int bonus)
{
    updateMainHist(board, move, bonus);
    updateContHist(move, movingPiece(board, move), contHistEntries, bonus);
}

void History::updateContHist(Move move, Piece movingPiece, std::span<CHEntry*> contHistEntries, int bonus)
{
    for (auto entry : contHistEntries)
        if (entry)
            updateContHist(move, movingPiece, entry, bonus);
}

void History::updateNoisyStats(const Board& board, Move move, int bonus)
{
    updateCaptHist(board, move, bonus);
}

void History::updateCorrHist(const Board& board, int bonus, int depth, Move prevMove, Piece prevPiece, ContCorrEntry* contCorr2)
{
    Color stm = board.sideToMove();
    uint64_t threatsKey = murmurHash3((board.threats() & board.pieces(stm)).value());
    int scaledBonus = bonus * CORR_HIST_SCALE;
    int weight = std::min(1 + depth, 16);

    auto& pawnEntry = m_PawnCorrHist[static_cast<int>(stm)][board.pawnKey().value % PAWN_CORR_HIST_ENTRIES];
    pawnEntry.update(scaledBonus, weight);

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

    if (prevPiece == Piece::NONE)
        prevPiece = makePiece(PieceType::PAWN, ~board.sideToMove());
    if (contCorr2)
    {
        auto& contCorr2Entry = (*contCorr2)[packPieceIndices(prevPiece)][prevMove.toSq().value()];
        contCorr2Entry.update(scaledBonus, weight);
    }
}

int History::getMainHist(Move move, Bitboard threats, Color color) const
{
    bool srcThreat = threats.has(move.fromSq());
    bool dstThreat = threats.has(move.toSq());
    return m_MainHist[static_cast<int>(color)][move.fromTo()][srcThreat][dstThreat];
}

int History::getContHist(Move move, Piece movingPiece, const CHEntry* entry) const
{
    return (*entry)[packPieceIndices(movingPiece)][move.toSq().value()];
}

int History::getCaptHist(const Board& board, Move move) const
{
    bool srcThreat = board.threats().has(move.fromSq());
    bool dstThreat = board.threats().has(move.toSq());
    PieceType capturedPT = getPieceType(capturedPiece(board, move));
    return m_CaptHist[static_cast<int>(capturedPT)][packPieceIndices(movingPiece(board, move))][move.toSq().value()][srcThreat][dstThreat];
}

void History::updateMainHist(const Board& board, Move move, int bonus)
{
    bool srcThreat = board.threats().has(move.fromSq());
    bool dstThreat = board.threats().has(move.toSq());
    m_MainHist[static_cast<int>(board.sideToMove())][move.fromTo()][srcThreat][dstThreat].update(bonus);
}

void History::updateContHist(Move move, Piece movingPiece, CHEntry* entry, int bonus)
{
    (*entry)[packPieceIndices(movingPiece)][move.toSq().value()].update(bonus);
}

void History::updateCaptHist(const Board& board, Move move, int bonus)
{
    bool srcThreat = board.threats().has(move.fromSq());
    bool dstThreat = board.threats().has(move.toSq());
    PieceType capturedPT = getPieceType(capturedPiece(board, move));
    m_CaptHist[static_cast<int>(capturedPT)][packPieceIndices(movingPiece(board, move))][move.toSq().value()][srcThreat][dstThreat].update(bonus);
}

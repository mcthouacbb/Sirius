#include "history.h"
#include "search.h"
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
    int bonus = search::histBonusQuadratic * depth * depth;
    bonus += search::histBonusLinear * depth;
    bonus -= search::histBonusOffset;
    // formula from berserk
    return std::min(search::maxHistBonus, bonus);
}

int historyMalus(int depth)
{
    int malus = search::histMalusQuadratic * depth * depth;
    malus += search::histMalusLinear * depth;
    malus -= search::histMalusOffset;
    // formula from berserk
    return std::min(search::maxHistMalus, malus);
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
    fillHistTable(m_ContCorrHist, 0);
}

int History::getQuietStats(
    Move move, Bitboard threats, Piece movingPiece, const SearchStack* stack, int ply) const
{
    int score = getMainHist(move, threats, getPieceColor(movingPiece));
    if (ply > 0 && stack[-1].contHistEntry != nullptr)
        score += getContHist(move, movingPiece, stack[-1].contHistEntry);
    if (ply > 1 && stack[-2].contHistEntry != nullptr)
        score += getContHist(move, movingPiece, stack[-2].contHistEntry);
    if (ply > 3 && stack[-4].contHistEntry != nullptr)
        score += getContHist(move, movingPiece, stack[-4].contHistEntry);
    return score;
}

int History::getNoisyStats(const Board& board, Move move) const
{
    return getCaptHist(board, move);
}

int History::correctStaticEval(const Board& board, int staticEval, const SearchStack* stack, int ply) const
{
    Color stm = board.sideToMove();
    uint64_t threatsKey = murmurHash3((board.threats() & board.pieces(stm)).value());
    int pawnEntry = m_PawnCorrHist[static_cast<int>(stm)][board.pawnKey().value % CORR_HIST_ENTRIES];
    int nonPawnStmEntry = m_NonPawnCorrHist[static_cast<int>(stm)][static_cast<int>(stm)]
                                           [board.nonPawnKey(stm).value % CORR_HIST_ENTRIES];
    int nonPawnNstmEntry = m_NonPawnCorrHist[static_cast<int>(stm)][static_cast<int>(~stm)]
                                            [board.nonPawnKey(~stm).value % CORR_HIST_ENTRIES];
    int threatsEntry = m_ThreatsCorrHist[static_cast<int>(stm)][threatsKey % CORR_HIST_ENTRIES];
    int minorPieceEntry =
        m_MinorPieceCorrHist[static_cast<int>(stm)][board.minorPieceKey().value % CORR_HIST_ENTRIES];
    int majorPieceEntry =
        m_MajorPieceCorrHist[static_cast<int>(stm)][board.majorPieceKey().value % CORR_HIST_ENTRIES];

    int correction = 0;
    correction += search::pawnCorrWeight * pawnEntry;
    correction += search::nonPawnStmCorrWeight * nonPawnStmEntry;
    correction += search::nonPawnNstmCorrWeight * nonPawnNstmEntry;
    correction += search::threatsCorrWeight * threatsEntry;
    correction += search::minorCorrWeight * minorPieceEntry;
    correction += search::majorCorrWeight * majorPieceEntry;

    Move prevMove = ply > 0 ? stack[-1].playedMove : Move::nullmove();
    // use pawn to a1 as sentinel for null moves in contcorrhist
    Piece prevPiece = ply > 0 ? stack[-1].movedPiece : Piece::NONE;
    if (prevPiece == Piece::NONE)
        prevPiece = makePiece(PieceType::PAWN, ~board.sideToMove());

    const auto contCorrEntry = [&](int pliesBack)
    {
        int value = 0;
        if (ply >= pliesBack && stack[-pliesBack].contCorrEntry != nullptr)
            value =
                (*stack[-pliesBack].contCorrEntry)[packPieceIndices(prevPiece)][prevMove.toSq().value()];
        return value;
    };

    correction += search::contCorr2Weight * contCorrEntry(2);
    correction += search::contCorr3Weight * contCorrEntry(3);
    correction += search::contCorr4Weight * contCorrEntry(4);
    correction += search::contCorr5Weight * contCorrEntry(5);
    correction += search::contCorr6Weight * contCorrEntry(6);
    correction += search::contCorr7Weight * contCorrEntry(7);

    int corrected = staticEval + correction / (256 * CORR_HIST_SCALE);
    return std::clamp(corrected, -SCORE_MATE_IN_MAX + 1, SCORE_MATE_IN_MAX - 1);
}

void History::updateQuietStats(const Board& board, Move move, const SearchStack* stack, int ply, int bonus)
{
    updateMainHist(board, move, bonus);
    updateContHist(move, movingPiece(board, move), stack, ply, bonus);
}

void History::updateMainHist(Move move, Bitboard threats, Color color, int bonus)
{
    bool srcThreat = threats.has(move.fromSq());
    bool dstThreat = threats.has(move.toSq());
    m_MainHist[static_cast<int>(color)][move.fromTo()][srcThreat][dstThreat].update(bonus);
}

void History::updateContHist(Move move, Piece movingPiece, const SearchStack* stack, int ply, int bonus)
{
    if (ply > 0 && stack[-1].contHistEntry != nullptr)
        updateContHist(move, movingPiece, stack[-1].contHistEntry, bonus);
    if (ply > 1 && stack[-2].contHistEntry != nullptr)
        updateContHist(move, movingPiece, stack[-2].contHistEntry, bonus);
    if (ply > 3 && stack[-4].contHistEntry != nullptr)
        updateContHist(move, movingPiece, stack[-4].contHistEntry, bonus);
}

void History::updateNoisyStats(const Board& board, Move move, int bonus)
{
    updateCaptHist(board, move, bonus);
}

void History::updateCorrHist(const Board& board, int bonus, int depth, const SearchStack* stack, int ply)
{
    Color stm = board.sideToMove();
    uint64_t threatsKey = murmurHash3((board.threats() & board.pieces(stm)).value());
    int scaledBonus = bonus * CORR_HIST_SCALE;
    int weight = 2 * std::min(1 + depth, 16);

    auto& pawnEntry = m_PawnCorrHist[static_cast<int>(stm)][board.pawnKey().value % CORR_HIST_ENTRIES];
    pawnEntry.update(scaledBonus, weight);

    auto& nonPawnStmEntry = m_NonPawnCorrHist[static_cast<int>(stm)][static_cast<int>(stm)]
                                             [board.nonPawnKey(stm).value % CORR_HIST_ENTRIES];
    nonPawnStmEntry.update(scaledBonus, weight);

    auto& nonPawnNstmEntry = m_NonPawnCorrHist[static_cast<int>(stm)][static_cast<int>(~stm)]
                                              [board.nonPawnKey(~stm).value % CORR_HIST_ENTRIES];
    nonPawnNstmEntry.update(scaledBonus, weight);

    auto& threatsEntry = m_ThreatsCorrHist[static_cast<int>(stm)][threatsKey % CORR_HIST_ENTRIES];
    threatsEntry.update(scaledBonus, weight);

    auto& minorPieceEntry =
        m_MinorPieceCorrHist[static_cast<int>(stm)][board.minorPieceKey().value % CORR_HIST_ENTRIES];
    minorPieceEntry.update(scaledBonus, weight);

    auto& majorPieceEntry =
        m_MajorPieceCorrHist[static_cast<int>(stm)][board.majorPieceKey().value % CORR_HIST_ENTRIES];
    majorPieceEntry.update(scaledBonus, weight);

    Move prevMove = ply > 0 ? stack[-1].playedMove : Move::nullmove();
    // use pawn to a1 as sentinel for null moves in contcorrhist
    Piece prevPiece = ply > 0 ? stack[-1].movedPiece : Piece::NONE;
    if (prevPiece == Piece::NONE)
        prevPiece = makePiece(PieceType::PAWN, ~board.sideToMove());

    const auto updateContCorr = [&](int pliesBack)
    {
        if (ply >= pliesBack && stack[-pliesBack].contCorrEntry != nullptr)
        {
            auto& contCorrEntry =
                (*stack[-pliesBack].contCorrEntry)[packPieceIndices(prevPiece)][prevMove.toSq().value()];
            contCorrEntry.update(scaledBonus, weight);
        }
    };

    updateContCorr(2);
    updateContCorr(3);
    updateContCorr(4);
    updateContCorr(5);
    updateContCorr(6);
    updateContCorr(7);
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
    return m_CaptHist[static_cast<int>(capturedPT)][packPieceIndices(movingPiece(board, move))]
                     [move.toSq().value()][srcThreat][dstThreat];
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
    m_CaptHist[static_cast<int>(capturedPT)][packPieceIndices(movingPiece(board, move))]
              [move.toSq().value()][srcThreat][dstThreat]
                  .update(bonus);
}

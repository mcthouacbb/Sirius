#include "history.h"
#include "search.h"
#include "zobrist.h"

#include <cmath>

int historyBonus(int depth)
{
    int bonus = search::histBonusQuadratic * depth * depth / 64;
    bonus += search::histBonusLinear * depth;
    bonus -= search::histBonusOffset;
    // formula from berserk
    return std::min(search::maxHistBonus, bonus);
}

int historyMalus(int depth)
{
    int malus = search::histMalusQuadratic * depth * depth / 64;
    malus += search::histMalusLinear * depth;
    malus -= search::histMalusOffset;
    // formula from berserk
    return std::min(search::maxHistMalus, malus);
}

void History::clear()
{
    std::memset(&m_MainHist, 0, sizeof(m_MainHist));
    std::memset(&m_PawnHist, 0, sizeof(m_PawnHist));
    std::memset(&m_ContHist, 0, sizeof(m_ContHist));
    std::memset(&m_CaptHist, 0, sizeof(m_CaptHist));
    std::memset(&m_PawnCorrHist, 0, sizeof(m_PawnCorrHist));
    std::memset(&m_NonPawnCorrHist, 0, sizeof(m_NonPawnCorrHist));
    std::memset(&m_ThreatsCorrHist, 0, sizeof(m_ThreatsCorrHist));
    std::memset(&m_MinorPieceCorrHist, 0, sizeof(m_MinorPieceCorrHist));
    std::memset(&m_MajorPieceCorrHist, 0, sizeof(m_MajorPieceCorrHist));
    std::memset(&m_ContCorrHist, 0, sizeof(m_ContCorrHist));
}

int History::getQuietStats(Move move, Bitboard threats, Piece movingPiece, ZKey pawnKey,
    const SearchStack* stack, int ply) const
{
    int score = getMainHist(move, threats, getPieceColor(movingPiece));
    score += getPawnHist(move, movingPiece, pawnKey);
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
    int pawnEntry = m_PawnCorrHist.get(stm, board.pawnKey().value);
    int nonPawnStmEntry = m_NonPawnCorrHist[stm].get(stm, board.nonPawnKey(stm).value);
    int nonPawnNstmEntry = m_NonPawnCorrHist[stm].get(~stm, board.nonPawnKey(~stm).value);
    int threatsEntry = m_ThreatsCorrHist.get(stm, threatsKey);
    int minorPieceEntry = m_MinorPieceCorrHist.get(stm, board.minorPieceKey().value);
    int majorPieceEntry = m_MajorPieceCorrHist.get(stm, board.majorPieceKey().value);

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
    updatePawnHist(board, move, bonus);
    updateContHist(move, movingPiece(board, move), stack, ply, bonus);
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

void History::updateCorrHist(
    const Board& board, int bonus, int depth, int complexity, const SearchStack* stack, int ply)
{
    Color stm = board.sideToMove();
    uint64_t threatsKey = murmurHash3((board.threats() & board.pieces(stm)).value());
    int scaledBonus = bonus * CORR_HIST_SCALE;
    float weight = 2 * std::min(1 + depth, 16) / 256.0f;
    weight *= 1.0f + std::log2(static_cast<float>(complexity + 1)) / 10.0f;

    auto& pawnEntry = m_PawnCorrHist.get(stm, board.pawnKey().value);
    pawnEntry.update(scaledBonus, weight);

    auto& nonPawnStmEntry = m_NonPawnCorrHist[stm].get(stm, board.nonPawnKey(stm).value);
    nonPawnStmEntry.update(scaledBonus, weight);

    auto& nonPawnNstmEntry = m_NonPawnCorrHist[stm].get(~stm, board.nonPawnKey(~stm).value);
    nonPawnNstmEntry.update(scaledBonus, weight);

    auto& threatsEntry = m_ThreatsCorrHist.get(stm, threatsKey);
    threatsEntry.update(scaledBonus, weight);

    auto& minorPieceEntry = m_MinorPieceCorrHist.get(stm, board.minorPieceKey().value);
    minorPieceEntry.update(scaledBonus, weight);

    auto& majorPieceEntry = m_MajorPieceCorrHist.get(stm, board.majorPieceKey().value);
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

int History::getPawnHist(Move move, Piece movingPiece, ZKey pawnKey) const
{
    return m_PawnHist[pawnKey.value % PAWN_HIST_ENTRIES][packPieceIndices(movingPiece)][move.toSq().value()];
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

void History::updatePawnHist(const Board& board, Move move, int bonus)
{
    m_PawnHist[board.pawnKey().value % PAWN_HIST_ENTRIES]
              [packPieceIndices(movingPiece(board, move))][move.toSq().value()]
                  .update(bonus);
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

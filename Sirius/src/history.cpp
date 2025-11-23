#include "history.h"
#include "search.h"
#include "zobrist.h"

i32 historyBonus(i32 depth)
{
    i32 bonus = search::histBonusQuadratic * depth * depth / 64;
    bonus += search::histBonusLinear * depth;
    bonus -= search::histBonusOffset;
    // formula from berserk
    return std::min(search::maxHistBonus, bonus);
}

i32 historyMalus(i32 depth)
{
    i32 malus = search::histMalusQuadratic * depth * depth / 64;
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

i32 History::getQuietStats(Move move, Bitboard threats, Piece movingPiece, ZKey pawnKey,
    const SearchStack* stack, i32 ply) const
{
    i32 score = getMainHist(move, threats, getPieceColor(movingPiece));
    score += getPawnHist(move, movingPiece, pawnKey);
    if (ply > 0 && stack[-1].contHistEntry != nullptr)
        score += getContHist(move, movingPiece, stack[-1].contHistEntry);
    if (ply > 1 && stack[-2].contHistEntry != nullptr)
        score += getContHist(move, movingPiece, stack[-2].contHistEntry);
    if (ply > 3 && stack[-4].contHistEntry != nullptr)
        score += getContHist(move, movingPiece, stack[-4].contHistEntry);
    return score;
}

i32 History::getNoisyStats(const Board& board, Move move) const
{
    return getCaptHist(board, move);
}

i32 History::correctStaticEval(const Board& board, i32 staticEval, const SearchStack* stack, i32 ply) const
{
    Color stm = board.sideToMove();
    u64 threatsKey = murmurHash3((board.threats() & board.pieces(stm)).value());
    i32 pawnEntry = m_PawnCorrHist.get(stm, board.pawnKey().value);
    i32 nonPawnStmEntry = m_NonPawnCorrHist[stm].get(stm, board.nonPawnKey(stm).value);
    i32 nonPawnNstmEntry = m_NonPawnCorrHist[stm].get(~stm, board.nonPawnKey(~stm).value);
    i32 threatsEntry = m_ThreatsCorrHist.get(stm, threatsKey);
    i32 minorPieceEntry = m_MinorPieceCorrHist.get(stm, board.minorPieceKey().value);
    i32 majorPieceEntry = m_MajorPieceCorrHist.get(stm, board.majorPieceKey().value);

    i32 correction = 0;
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

    const auto contCorrEntry = [&](i32 pliesBack)
    {
        i32 value = 0;
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

    i32 corrected = staticEval + correction / (256 * CORR_HIST_SCALE);
    return std::clamp(corrected, -SCORE_MATE_IN_MAX + 1, SCORE_MATE_IN_MAX - 1);
}

void History::updateQuietStats(const Board& board, Move move, const SearchStack* stack, i32 ply, i32 bonus)
{
    updateMainHist(board, move, bonus);
    updatePawnHist(board, move, bonus);
    updateContHist(move, board.threats(), movingPiece(board, move), stack, ply, bonus);
}

void History::updateContHist(Move move, Bitboard threats, Piece movingPiece, const SearchStack* stack, i32 ply, i32 bonus)
{
    i32 histBase = 0;
    histBase += getMainHist(move, threats, getPieceColor(movingPiece)) / 2;
    if (ply > 0 && stack[-1].contHistEntry != nullptr)
        histBase += getContHist(move, movingPiece, stack[-1].contHistEntry);
    if (ply > 1 && stack[-2].contHistEntry != nullptr)
        histBase += getContHist(move, movingPiece, stack[-2].contHistEntry);
    if (ply > 3 && stack[-4].contHistEntry != nullptr)
        histBase += getContHist(move, movingPiece, stack[-4].contHistEntry);

    if (ply > 0 && stack[-1].contHistEntry != nullptr)
        updateContHist(move, movingPiece, stack[-1].contHistEntry, histBase, bonus);
    if (ply > 1 && stack[-2].contHistEntry != nullptr)
        updateContHist(move, movingPiece, stack[-2].contHistEntry, histBase, bonus);
    if (ply > 3 && stack[-4].contHistEntry != nullptr)
        updateContHist(move, movingPiece, stack[-4].contHistEntry, histBase, bonus);
}

void History::updateNoisyStats(const Board& board, Move move, i32 bonus)
{
    updateCaptHist(board, move, bonus);
}

void History::updateCorrHist(const Board& board, i32 bonus, i32 depth, const SearchStack* stack, i32 ply)
{
    Color stm = board.sideToMove();
    u64 threatsKey = murmurHash3((board.threats() & board.pieces(stm)).value());
    i32 scaledBonus = bonus * CORR_HIST_SCALE;
    i32 weight = 2 * std::min(1 + depth, 16);

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

    const auto updateContCorr = [&](i32 pliesBack)
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

i32 History::getMainHist(Move move, Bitboard threats, Color color) const
{
    bool srcThreat = threats.has(move.fromSq());
    bool dstThreat = threats.has(move.toSq());
    return m_MainHist[static_cast<i32>(color)][move.fromTo()][srcThreat][dstThreat];
}

i32 History::getPawnHist(Move move, Piece movingPiece, ZKey pawnKey) const
{
    return m_PawnHist[pawnKey.value % PAWN_HIST_ENTRIES][packPieceIndices(movingPiece)][move.toSq().value()];
}

i32 History::getContHist(Move move, Piece movingPiece, const CHEntry* entry) const
{
    return (*entry)[packPieceIndices(movingPiece)][move.toSq().value()];
}

i32 History::getCaptHist(const Board& board, Move move) const
{
    bool srcThreat = board.threats().has(move.fromSq());
    bool dstThreat = board.threats().has(move.toSq());
    PieceType capturedPT = getPieceType(capturedPiece(board, move));
    return m_CaptHist[static_cast<i32>(capturedPT)][packPieceIndices(movingPiece(board, move))]
                     [move.toSq().value()][srcThreat][dstThreat];
}

void History::updateMainHist(const Board& board, Move move, i32 bonus)
{
    bool srcThreat = board.threats().has(move.fromSq());
    bool dstThreat = board.threats().has(move.toSq());
    m_MainHist[static_cast<i32>(board.sideToMove())][move.fromTo()][srcThreat][dstThreat].update(bonus);
}

void History::updatePawnHist(const Board& board, Move move, i32 bonus)
{
    m_PawnHist[board.pawnKey().value % PAWN_HIST_ENTRIES]
              [packPieceIndices(movingPiece(board, move))][move.toSq().value()]
                  .update(bonus);
}

void History::updateContHist(Move move, Piece movingPiece, CHEntry* entry, i32 base, i32 bonus)
{
    auto& contHistEntry = (*entry)[packPieceIndices(movingPiece)][move.toSq().value()];
    contHistEntry.update(bonus, base);
}

void History::updateCaptHist(const Board& board, Move move, i32 bonus)
{
    bool srcThreat = board.threats().has(move.fromSq());
    bool dstThreat = board.threats().has(move.toSq());
    PieceType capturedPT = getPieceType(capturedPiece(board, move));
    m_CaptHist[static_cast<i32>(capturedPT)][packPieceIndices(movingPiece(board, move))]
              [move.toSq().value()][srcThreat][dstThreat]
                  .update(bonus);
}

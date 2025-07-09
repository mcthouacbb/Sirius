#include "eval_state.h"
#include "../board.h"
#include "eval_terms.h"
#include <algorithm>

namespace eval
{

EvalState::EvalState()
{
    std::fill(m_Stack.begin(), m_Stack.end(), StackEntry{});
    m_CurrEntry = &m_Stack[0];
}

void EvalState::initSingle(const Board& board)
{
    init(board, nullptr);
}

void EvalState::init(const Board& board, PawnTable& pawnTable)
{
    init(board, &pawnTable);
}

void EvalState::init(const Board& board, PawnTable* pawnTable)
{
    using enum PieceType;
    using enum Color;

    std::fill(m_Stack.begin(), m_Stack.end(), StackEntry{});
    m_CurrEntry = &m_Stack[0];
    m_PawnTable = pawnTable;

    currEntry().psqtState.init();
    for (Color c : {WHITE, BLACK})
    {
        for (PieceType pt : {PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING})
        {
            Bitboard pieces = board.pieces(c, pt);
            while (pieces.any())
                currEntry().psqtState.addPiece(c, pt, pieces.poplsb());
        }
    }

    evaluatePawns(board, currEntry().pawnStructure.data, m_PawnTable);
    currEntry().pawnStructure.updated = true;

    currEntry().pawnShieldStorm.data[WHITE] = evaluateStormShield<WHITE>(board);
    currEntry().pawnShieldStorm.data[BLACK] = evaluateStormShield<BLACK>(board);
    currEntry().pawnShieldStorm.updated = true;

    currEntry().knightOutposts.data =
        evaluateKnightOutposts<WHITE>(board, currEntry().pawnStructure.data)
        - evaluateKnightOutposts<BLACK>(board, currEntry().pawnStructure.data);
    currEntry().knightOutposts.updated = true;

    currEntry().bishopPawns.data =
        evaluateBishopPawns<WHITE>(board) - evaluateBishopPawns<BLACK>(board);
    currEntry().bishopPawns.updated = true;

    currEntry().rookOpen.data = evaluateRookOpen<WHITE>(board) - evaluateRookOpen<BLACK>(board);
    currEntry().rookOpen.updated = true;

    currEntry().minorBehindPawn.data =
        evaluateMinorBehindPawn<WHITE>(board) - evaluateMinorBehindPawn<BLACK>(board);
    currEntry().minorBehindPawn.updated = true;
}

void EvalState::push(const EvalUpdates& updates)
{
    const auto& oldEntry = currEntry();
    m_CurrEntry++;

    assert(m_CurrEntry < m_Stack.data() + m_Stack.size());

    currEntry().updates = updates;

    currEntry().psqtState = oldEntry.psqtState;
    for (const auto& add : currEntry().updates.adds)
    {
        currEntry().psqtState.addPiece(getPieceColor(add.piece), getPieceType(add.piece), add.square);
    }

    for (const auto& add : currEntry().updates.removes)
    {
        currEntry().psqtState.removePiece(getPieceColor(add.piece), getPieceType(add.piece), add.square);
    }

    if (oldEntry.pawnStructure.updated && !updates.changedPieces.hasAny(eval_terms::pawnStructure.deps))
        currEntry().pawnStructure = oldEntry.pawnStructure;
    else
        currEntry().pawnStructure.updated = false;

    if (oldEntry.pawnShieldStorm.updated
        && !updates.changedPieces.hasAny(eval_terms::pawnShieldStorm.deps))
        currEntry().pawnShieldStorm = oldEntry.pawnShieldStorm;
    else
        currEntry().pawnShieldStorm.updated = false;

    if (oldEntry.knightOutposts.updated && !updates.changedPieces.hasAny(eval_terms::knightOutposts.deps))
        currEntry().knightOutposts = oldEntry.knightOutposts;
    else
        currEntry().knightOutposts.updated = false;

    if (oldEntry.bishopPawns.updated && !updates.changedPieces.hasAny(eval_terms::bishopPawns.deps))
        currEntry().bishopPawns = oldEntry.bishopPawns;
    else
        currEntry().bishopPawns.updated = false;

    if (oldEntry.rookOpen.updated && !updates.changedPieces.hasAny(eval_terms::rookOpen.deps))
        currEntry().rookOpen = oldEntry.rookOpen;
    else
        currEntry().rookOpen.updated = false;

    if (oldEntry.minorBehindPawn.updated
        && !updates.changedPieces.hasAny(eval_terms::minorBehindPawn.deps))
        currEntry().minorBehindPawn = oldEntry.minorBehindPawn;
    else
        currEntry().minorBehindPawn.updated = false;
}

void EvalState::pop()
{
    m_CurrEntry--;
}

void EvalState::update(const Board& board)
{
    using enum Color;

    if (!currEntry().pawnStructure.updated)
        evaluatePawns(board, currEntry().pawnStructure.data, m_PawnTable);

    if (!currEntry().pawnShieldStorm.updated)
    {
        currEntry().pawnShieldStorm.data[WHITE] = evaluateStormShield<WHITE>(board);
        currEntry().pawnShieldStorm.data[BLACK] = evaluateStormShield<BLACK>(board);
    }

    if (!currEntry().knightOutposts.updated)
        currEntry().knightOutposts.data =
            evaluateKnightOutposts<WHITE>(board, currEntry().pawnStructure.data)
            - evaluateKnightOutposts<BLACK>(board, currEntry().pawnStructure.data);

    if (!currEntry().bishopPawns.updated)
        currEntry().bishopPawns.data =
            evaluateBishopPawns<WHITE>(board) - evaluateBishopPawns<BLACK>(board);

    if (!currEntry().rookOpen.updated)
        currEntry().rookOpen.data = evaluateRookOpen<WHITE>(board) - evaluateRookOpen<BLACK>(board);

    if (!currEntry().minorBehindPawn.updated)
        currEntry().minorBehindPawn.data =
            evaluateMinorBehindPawn<WHITE>(board) - evaluateMinorBehindPawn<BLACK>(board);
}

ScorePair EvalState::score(const Board& board) const
{
    return currEntry().psqtState.evaluate(board) + currEntry().pawnStructure.data.score
        + currEntry().knightOutposts.data + currEntry().bishopPawns.data + currEntry().rookOpen.data
        + currEntry().minorBehindPawn.data;
}

ScorePair EvalState::psqtScore(const Board& board, Color c) const
{
    auto psqt = currEntry().psqtState.evaluate(board);
    if (c == Color::BLACK)
        return -psqt;
    return psqt;
}

ScorePair EvalState::pawnShieldStormScore(Color c) const
{
    return currEntry().pawnShieldStorm.data[c];
}

const PawnStructure& EvalState::pawnStructure() const
{
    return currEntry().pawnStructure.data;
}

}

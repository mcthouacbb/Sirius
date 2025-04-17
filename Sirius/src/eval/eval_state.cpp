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
    // m_PawnTable = pawnTable;

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

    /*evaluatePawns(board, currEntry().pawnStructure, m_PawnTable);
    currEntry().pawnShieldStorm[WHITE] = evaluateStormShield<WHITE>(board);
    currEntry().pawnShieldStorm[BLACK] = evaluateStormShield<BLACK>(board);
    currEntry().knightOutposts = evaluateKnightOutposts<WHITE>(board, currEntry().pawnStructure)
        - evaluateKnightOutposts<BLACK>(board, currEntry().pawnStructure);
    currEntry().bishopPawns = evaluateBishopPawns<WHITE>(board) - evaluateBishopPawns<BLACK>(board);
    currEntry().rookOpen = evaluateRookOpen<WHITE>(board) - evaluateRookOpen<BLACK>(board);
    currEntry().minorBehindPawn =
        evaluateMinorBehindPawn<WHITE>(board) - evaluateMinorBehindPawn<BLACK>(board);*/
}

void EvalState::push(const Board& board, const EvalUpdates& updates)
{
    using enum Color;

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

    /*if (updates.changedPieces.hasAny(eval_terms::pawnStructure.deps))
        evaluatePawns(board, currEntry().pawnStructure, m_PawnTable);
    else
        currEntry().pawnStructure = oldEntry.pawnStructure;

    if (updates.changedPieces.hasAny(eval_terms::pawnShieldStorm.deps))
    {
        currEntry().pawnShieldStorm[WHITE] = evaluateStormShield<WHITE>(board);
        currEntry().pawnShieldStorm[BLACK] = evaluateStormShield<BLACK>(board);
    }
    else
        currEntry().pawnShieldStorm = oldEntry.pawnShieldStorm;

    if (updates.changedPieces.hasAny(eval_terms::knightOutposts.deps))
        currEntry().knightOutposts = evaluateKnightOutposts<WHITE>(board, currEntry().pawnStructure)
            - evaluateKnightOutposts<BLACK>(board, currEntry().pawnStructure);
    else
        currEntry().knightOutposts = oldEntry.knightOutposts;

    if (updates.changedPieces.hasAny(eval_terms::bishopPawns.deps))
        currEntry().bishopPawns = evaluateBishopPawns<WHITE>(board) -
    evaluateBishopPawns<BLACK>(board); else currEntry().bishopPawns = oldEntry.bishopPawns;

    if (updates.changedPieces.hasAny(eval_terms::rookOpen.deps))
        currEntry().rookOpen = evaluateRookOpen<WHITE>(board) - evaluateRookOpen<BLACK>(board);
    else
        currEntry().rookOpen = oldEntry.rookOpen;

    if (updates.changedPieces.hasAny(eval_terms::minorBehindPawn.deps))
        currEntry().minorBehindPawn =
            evaluateMinorBehindPawn<WHITE>(board) - evaluateMinorBehindPawn<BLACK>(board);
    else
        currEntry().minorBehindPawn = oldEntry.minorBehindPawn;*/
}

void EvalState::pop()
{
    m_CurrEntry--;
}

ScorePair EvalState::score(const Board& board) const
{
    return currEntry().psqtState.evaluate(board); /* + currEntry().pawnStructure.score
         + currEntry().knightOutposts + currEntry().bishopPawns + currEntry().rookOpen
         + currEntry().minorBehindPawn;*/
}

ScorePair EvalState::psqtScore(const Board& board, Color c) const
{
    auto psqt = currEntry().psqtState.evaluate(board);
    if (c == Color::BLACK)
        return -psqt;
    return psqt;
}

/*ScorePair EvalState::pawnShieldStormScore(Color c) const
{
    return currEntry().pawnShieldStorm[c];
}

const PawnStructure& EvalState::pawnStructure() const
{
    return currEntry().pawnStructure;
}*/

}

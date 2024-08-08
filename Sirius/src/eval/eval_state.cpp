#include "eval_state.h"
#include "eval_terms.h"
#include "../board.h"
#include <algorithm>

namespace eval
{

EvalState::EvalState()
{
    std::fill(m_Stack.begin(), m_Stack.end(), StackEntry{});
    m_CurrEntry = &m_Stack[0];
}

void EvalState::init(const Board& board)
{
    currEntry().psqtState.init();
    for (Color c : {Color::WHITE, Color::BLACK})
        for (PieceType pt : {PieceType::PAWN, PieceType::KNIGHT, PieceType::BISHOP, PieceType::ROOK, PieceType::QUEEN, PieceType::KING})
        {
            Bitboard pieces = board.pieces(c, pt);
            while (pieces.any())
                currEntry().psqtState.addPiece(c, pt, pieces.poplsb());
        }

    currEntry().pawnStructure = PawnStructure(board);
    currEntry().passers = {};// evaluatePassers
    currEntry().pawnShieldStorm = {};// evaluatePawnShieldStorm
    currEntry().knightOutposts = {};// evaluateKnightOutposts
    currEntry().bishopPawns = {};// evaluateBishopPawns
    currEntry().rookOpen = {};// evaluateRookOpen
}

void EvalState::push(const Board& board, const EvalUpdates& updates)
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

    if (updates.changedPieces.hasAny(eval_terms::pawnStructure.deps))
        currEntry().pawnStructure = PawnStructure(board);
    else
        currEntry().pawnStructure = oldEntry.pawnStructure;

    if (updates.changedPieces.hasAny(eval_terms::passers.deps))
        currEntry().passers = {};// evaluatePassers
    else
        currEntry().passers = oldEntry.passers;

    if (updates.changedPieces.hasAny(eval_terms::pawnShieldStorm.deps))
        currEntry().pawnShieldStorm = {};// evaluatePawnShieldStorm
    else
        currEntry().pawnShieldStorm = oldEntry.pawnShieldStorm;

    if (updates.changedPieces.hasAny(eval_terms::knightOutposts.deps))
        currEntry().knightOutposts = {};// evaluateKnightOutposts
    else
        currEntry().knightOutposts = oldEntry.knightOutposts;

    if (updates.changedPieces.hasAny(eval_terms::bishopPawns.deps))
        currEntry().bishopPawns = {};// evaluateBishopPawns
    else
        currEntry().bishopPawns = oldEntry.bishopPawns;

    if (updates.changedPieces.hasAny(eval_terms::rookOpen.deps))
        currEntry().rookOpen = {};// evaluateRookOpen
    else
        currEntry().rookOpen = oldEntry.rookOpen;
}

void EvalState::pop()
{
    m_CurrEntry--;
}


}

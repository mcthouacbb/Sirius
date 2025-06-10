#pragma once

#include "../bitboard.h"
#include "../defs.h"
#include "eval_state.h"
#include <bitset>
#include <type_traits>

class Board;
struct PawnStructure;
class PawnTable;

namespace eval
{

namespace eval_terms
{

using enum PieceType;

inline bool pawnStructureChanged(const EvalUpdates& updates)
{
    return updates.changedPieces.has(PieceType::PAWN);
}

inline bool pawnShieldStormChanged(const EvalUpdates& updates)
{
    return updates.changedPieces.hasAny(PieceSet(PieceType::PAWN, PieceType::KING));
}

inline bool knightOutpostsChanged(const EvalUpdates& updates)
{
    return updates.changedPieces.hasAny(PieceSet(PieceType::PAWN, PieceType::KNIGHT));
}

inline bool bishopPawnsChanged(const EvalUpdates& updates)
{
    return updates.changedPieces.hasAny(PieceSet(PieceType::PAWN, PieceType::BISHOP));
}

inline bool rookOpenChanged(const EvalUpdates& updates)
{
    return updates.changedPieces.hasAny(PieceSet(PieceType::PAWN, PieceType::ROOK));
}

inline bool minorBehindPawnChanged(const EvalUpdates& updates)
{
    return updates.changedPieces.hasAny(
        PieceSet(PieceType::PAWN, PieceType::KNIGHT, PieceType::BISHOP));
}

} // namespace eval_terms

void evaluatePawns(const Board& board, PawnStructure& pawnStructure, PawnTable* pawnTable);

template<Color us>
ScorePair evalKingPawnFile(uint32_t file, Bitboard ourPawns, Bitboard theirPawns);

template<Color us>
ScorePair evaluateStormShield(const Board& board);

template<Color us>
ScorePair evaluateKnightOutposts(const Board& board, const PawnStructure& pawnStructure);

template<Color us>
ScorePair evaluateBishopPawns(const Board& board);

template<Color us>
ScorePair evaluateRookOpen(const Board& board);

template<Color us>
ScorePair evaluateMinorBehindPawn(const Board& board);

}

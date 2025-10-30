#pragma once

#include "../bitboard.h"
#include "../defs.h"
#include "../util/piece_set.h"
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

struct EvalTerm
{
    PieceSet deps;
};

constexpr EvalTerm pawnStructure = {PieceSet(PAWN)};
constexpr EvalTerm pawnShieldStorm = {PieceSet(PAWN, KING)};
constexpr EvalTerm outposts = {PieceSet(PAWN, KNIGHT, BISHOP)};
constexpr EvalTerm bishopPawns = {PieceSet(PAWN, BISHOP)};
constexpr EvalTerm rookOpen = {PieceSet(PAWN, ROOK)};
constexpr EvalTerm minorBehindPawn = {PieceSet(PAWN, KNIGHT, BISHOP)};

} // namespace eval_terms

void evaluatePawns(const Board& board, PawnStructure& pawnStructure, PawnTable* pawnTable);

template<Color us>
ScorePair evalKingPawnFile(uint32_t file, Bitboard ourPawns, Bitboard theirPawns);

template<Color us>
ScorePair evaluateStormShield(const Board& board);

template<Color us>
ScorePair evaluateOutposts(const Board& board, const PawnStructure& pawnStructure);

template<Color us>
ScorePair evaluateBishopPawns(const Board& board);

template<Color us>
ScorePair evaluateRookOpen(const Board& board);

template<Color us>
ScorePair evaluateMinorBehindPawn(const Board& board);

}

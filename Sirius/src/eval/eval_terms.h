#pragma once

#include <bitset>
#include <type_traits>
#include "../defs.h"
#include "../bitboard.h"
#include "../util/piece_set.h"

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
constexpr EvalTerm passers = {PieceSet(PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING)};
constexpr EvalTerm pawnShieldStorm = {PieceSet(PAWN, KING)};
constexpr EvalTerm knightOutposts = {PieceSet(PAWN, KNIGHT)};
constexpr EvalTerm bishopPawns = {PieceSet(PAWN, BISHOP)};
constexpr EvalTerm rookOpen = {PieceSet(PAWN, ROOK)};
constexpr EvalTerm minorBehindPawn = {PieceSet(PAWN, KNIGHT, BISHOP)};


} // namespace eval_terms

void evaluatePawns(const Board& board, PawnStructure& pawnStructure, PawnTable* pawnTable);

template<Color us>
PackedScore evalKingPawnFile(uint32_t file, Bitboard ourPawns, Bitboard theirPawns);

template<Color us>
PackedScore evaluateStormShield(const Board& board);

template<Color us>
PackedScore evaluateKnightOutposts(const Board& board, const PawnStructure& pawnStructure);

template<Color us>
PackedScore evaluateBishopPawns(const Board& board);

template<Color us>
PackedScore evaluateRookOpen(const Board& board);

template<Color us>
PackedScore evaluateMinorBehindPawn(const Board& board);


}

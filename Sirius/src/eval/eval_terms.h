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

struct EvalTerm
{
    PieceSet deps;
};

constexpr EvalTerm pawnStructure = {PieceSet(PieceType::PAWN)};
constexpr EvalTerm passers = {PieceSet(PieceType::PAWN, PieceType::KNIGHT, PieceType::BISHOP, PieceType::ROOK, PieceType::QUEEN, PieceType::KING)};
constexpr EvalTerm pawnShieldStorm = {PieceSet(PieceType::PAWN, PieceType::KING)};
constexpr EvalTerm knightOutposts = {PieceSet(PieceType::PAWN, PieceType::KNIGHT)};
constexpr EvalTerm bishopPawns = {PieceSet(PieceType::PAWN, PieceType::BISHOP)};
constexpr EvalTerm rookOpen = {PieceSet(PieceType::PAWN, PieceType::ROOK)};


} // namespace eval_terms

void evaluatePawns(const Board& board, PawnStructure& pawnStructure, PawnTable* pawnTable);

template<Color us>
PackedScore evaluatePassedPawns(const Board& board, const PawnStructure& pawnStructure);

template<Color us>
PackedScore evalKingPawnFile(uint32_t file, Bitboard ourPawns, Bitboard theirPawns, Square theirKing);

template<Color us>
PackedScore evaluateStormShield(const Board& board);

template<Color us>
PackedScore evaluateKnightOutposts(const Board& board, const PawnStructure& pawnStructure);

template<Color us>
PackedScore evaluateBishopPawns(const Board& board);

template<Color us>
PackedScore evaluateRookOpen(const Board& board);


}

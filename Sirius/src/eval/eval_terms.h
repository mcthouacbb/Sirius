#pragma once

#include <bitset>
#include <type_traits>
#include "../defs.h"
#include "../util/piece_set.h"

namespace eval
{

struct EvalTerm
{
    PieceSet deps;
};

namespace eval_terms
{

constexpr EvalTerm pawnStructure = {PieceSet(PieceType::PAWN)};
constexpr EvalTerm passers = {PieceSet(PieceType::PAWN, PieceType::KNIGHT, PieceType::BISHOP, PieceType::ROOK, PieceType::QUEEN, PieceType::KING)};
constexpr EvalTerm pawnShieldStorm = {PieceSet(PieceType::PAWN, PieceType::KING)};
constexpr EvalTerm knightOutposts = {PieceSet(PieceType::PAWN, PieceType::KNIGHT)};
constexpr EvalTerm bishopPawns = {PieceSet(PieceType::PAWN, PieceType::BISHOP)};
constexpr EvalTerm rookOpen = {PieceSet(PieceType::PAWN, PieceType::ROOK)};


}

}

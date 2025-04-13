#pragma once

#include "../board.h"
#include "../util/enum_array.h"

namespace eval
{

struct PawnStructure
{
    PawnStructure() = default;
    PawnStructure(const Board& board);

    PackedScore evaluate(const Board& board);

    ColorArray<Bitboard> pawnAttacks;
    ColorArray<Bitboard> pawnAttackSpans;
    ColorArray<Bitboard> pawnAttacks2;
    Bitboard passedPawns;
    Bitboard candidateBlockedPassers;
    PackedScore score;
private:
    template<Color us>
    PackedScore evaluate(const Board& board);
};


}

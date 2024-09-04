#pragma once

#include "board.h"
#include "util/static_vector.h"

enum class MoveGenType
{
    LEGAL,
    NOISY_QUIET,
    NOISY,
    QUIET
};

using MoveList = StaticVector<Move, 256>;

template<MoveGenType type>
void genMoves(const Board& board, MoveList& moves);

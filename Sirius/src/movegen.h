#pragma once

#include "board.h"

enum class MoveGenType
{
    LEGAL,
    CAPTURES
};

template<MoveGenType type>
Move* genMoves(const Board& board, Move* moves);

#pragma once

#include "material.h"
#include "phase.h"
#include "draw.h"
#include "../board.h"

namespace eval
{

void init();

inline bool isMateScore(int score)
{
    return std::abs(score) >= SCORE_MATE_IN_MAX;
}

int evaluate(const Board& board);
int rawEval(const Board& board);

}
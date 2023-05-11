#pragma once

#include "material.h"
#include "phase.h"
#include "draw.h"
#include "../board.h"

namespace eval
{

constexpr int NEG_INF = -1000000;
constexpr int POS_INF = 1000000;
constexpr int CHECKMATE = -32700;
constexpr int STALEMATE = 0;
constexpr int DRAW = 0;

void init();

inline bool isMateScore(int score)
{
	return std::abs(CHECKMATE) - std::abs(score) < 256;
}

int evaluate(const Board& board);
int rawEval(const Board& board);

}
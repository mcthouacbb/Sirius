#pragma once

#include "material.h"
#include "phase.h"
#include "../board.h"

namespace eval
{

constexpr int NEG_INF = -1000000;
constexpr int POS_INF = 1000000;
constexpr int CHECKMATE = -32700;
constexpr int STALEMATE = 0;

inline bool isMateScore(int score)
{
	return std::abs(CHECKMATE) - std::abs(score) < 256; 
}

int evalMaterialMG(const Board& board);
int evalMaterialEG(const Board& board);
int evaluate(const Board& board);

}
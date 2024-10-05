#pragma once

#include "phase.h"
#include "draw.h"
#include "../search.h"
#include "../board.h"

namespace eval
{

int evaluate(const Board& board, search::SearchThread* thread = nullptr);

int evaluateSingle(const Board& board);

}

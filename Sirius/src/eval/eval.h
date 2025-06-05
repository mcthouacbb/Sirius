#pragma once

#include "../board.h"
#include "../search.h"
#include "phase.h"

namespace eval
{

int evaluate(const Board& board, search::SearchThread* thread = nullptr);

int evaluateSingle(const Board& board);

}

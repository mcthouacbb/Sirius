#pragma once

#include "../board.h"
#include "../search.h"

namespace eval
{

i32 evaluate(const Board& board, search::SearchThread* thread = nullptr);

i32 evaluateSingle(const Board& board);

}

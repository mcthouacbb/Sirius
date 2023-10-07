#pragma once

#include "phase.h"
#include "draw.h"
#include "../board.h"

namespace eval
{

int evaluate(const Board& board);
int rawEval(const Board& board);

}

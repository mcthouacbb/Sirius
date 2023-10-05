#pragma once

#include "../board.h"

namespace eval
{

bool isImmediateDraw(const Board& board);
bool canForceMate(const Board& board);

}

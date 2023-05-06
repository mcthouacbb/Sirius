#pragma once

#include "../defs.h"

namespace eval
{

void initPSQT();

int getPSQTMG(PieceType piece, int square);
int getPSQTEG(PieceType piece, int square);

}
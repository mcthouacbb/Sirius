#pragma once

#include "board.h"
#include "movegen.h"

void printBoard(const Board& board);

template<bool print>
u64 perft(Board& board, i32 depth);

void testSAN(Board& board, i32 depth);

void testKeyAfter(Board& board, i32 depth);

void testNoisyGen(Board& board, i32 depth);
void testQuietGen(Board& board, i32 depth);

void testIsPseudoLegal(Board& board, i32 depth);

void testMarlinformat(Board& board, i32 depth);

void testSEE();

void runTests(Board& board, bool fast);

void testSANFind(const Board& board, const MoveList& moveList, i32 len);

#include "board.h"
#include "movegen.h"

void printBoard(const Board& board);

template<bool print>
uint64_t perft(Board& board, int depth);

void testSAN(Board& board, int depth);

void testKeyAfter(Board& board, int depth);

void testQuiescence(Board& board, int depth);

void testSEE();

void runTests(Board& board, bool fast);

void testSANFind(const Board& board, const MoveList& moveList, int len);

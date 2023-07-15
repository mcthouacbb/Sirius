#pragma once

#include "board.h"
#include "defs.h"

constexpr int MAX_PLY = 128;

struct SearchPly
{
	Move* pv;
	int pvLength;
	
};

class Search
{
public:
	Search(Board& board);

	int iterDeep(int maxDepth);

	int search(int depth, int alpha, int beta);
	int qsearch(int alpha, int beta, int depth);
private:
	Board& m_Board;
	int m_RootPly;
	uint64_t m_Nodes;
	uint64_t m_QNodes;
	Move m_PV[MAX_PLY];
	SearchPly m_Plies[MAX_PLY];
};


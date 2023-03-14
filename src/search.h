#pragma once

#include "board.h"
#include "defs.h"

constexpr int MAX_PLY = 128;

struct SearchPly
{
	Move* pv;
	int pvLength;
	Move killers[2];
};

class Search
{
public:
	Search(Board& board);

	int iterDeep(int maxDepth);

	int search(int depth, int alpha, int beta);
	int qsearch(int alpha, int beta);
private:
	void reset();
	void storeKiller(int ply, Move killer);

	Board& m_Board;
	int m_RootPly;
	uint64_t m_Nodes;
	uint64_t m_QNodes;
	Move m_PV[MAX_PLY];
	int m_History[2][4096];
	SearchPly m_Plies[MAX_PLY];
};


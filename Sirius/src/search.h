#pragma once

#include "board.h"
#include "defs.h"
#include "time_man.h"

constexpr int MAX_PLY = 128;

struct SearchPly
{
	Move* pv;
	int pvLength;
	
};

class Search
{
public:
	static constexpr uint32_t TIME_CHECK_INTERVAL = 2048;

	Search(Board& board);

	int iterDeep(int maxDepth);

	int search(int depth, int alpha, int beta);
	int qsearch(int alpha, int beta);

	void setTime(Duration clock, Duration inc);
private:
	Board& m_Board;
	TimeManager m_TimeMan;
	bool m_ShouldStop;
	uint32_t m_TimeCheckCounter;
	int m_RootPly;
	uint64_t m_Nodes;
	uint64_t m_QNodes;
	Move m_PV[MAX_PLY];
	SearchPly m_Plies[MAX_PLY];
};

inline void Search::setTime(Duration clock, Duration inc)
{
	m_TimeMan.setTimeLeft(clock, inc);
}

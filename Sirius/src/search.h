#pragma once

#include "board.h"
#include "defs.h"

#include "time_man.h"

constexpr int MAX_PLY = 128;

struct SearchPly
{
	Move* pv;
	int pvLength;
	Move bestMove;
	Move killers[2];
};

struct SearchInfo
{
	int depth;
	uint64_t nodes;
	Duration time;
	const Move* pvBegin, * pvEnd;
	int score;
};

enum class SearchPolicy
{
	INFINITE,
	FIXED_TIME,
	DYN_CLOCK
};

struct SearchLimits
{
	SearchPolicy policy;
	int maxDepth;
	union
	{
		Duration time;
		struct
		{
			Duration timeLeft[2];
			Duration increments[2];
		} clock;
	};
};

namespace comm
{

class UCI;

}

class Search
{
public:
	static constexpr uint32_t CHECK_INTERVAL = 2048;
	Search(Board& board);

	int iterDeep(int maxDepth);
	int iterDeepUCI(const SearchLimits& limits, comm::UCI& uci);

	int search(int depth, SearchPly* searchPly, int alpha, int beta, bool isPV);
	int qsearch(int alpha, int beta);
private:
	void reset();
	void storeKiller(SearchPly* ply, Move killer);

	Board& m_Board;
	// uci specific
	TimeManager m_TimeMan;
	uint32_t m_CheckCounter;
	comm::UCI* m_UCI;
	bool m_ShouldStop;

	int m_RootPly;
	uint64_t m_Nodes;
	uint64_t m_QNodes;
	Move m_PV[MAX_PLY + 1];
	int m_History[2][4096];
	SearchPly m_Plies[MAX_PLY + 1];
};


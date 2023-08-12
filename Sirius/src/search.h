#pragma once

#include "board.h"
#include "defs.h"
#include "tt.h"
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

class Search
{
public:
	static constexpr uint32_t CHECK_INTERVAL = 2048;
	static constexpr int FUTILITY_MARGIN = 250;

	Search(Board& board);

	int iterDeep(const SearchLimits& limits);
	int aspWindows(int depth, int prevScore);

	int search(int depth);
	int qsearch();

	const SearchInfo& info() const;
private:
	void reset();
	void storeKiller(SearchPly* ply, Move killer);
	int search(int depth, SearchPly* searchPly, int alpha, int beta, bool isPV);
	int qsearch(SearchPly* searchPly, int alpha, int beta);

	Board& m_Board;
	TimeManager m_TimeMan;
	bool m_ShouldStop;
	uint32_t m_CheckCounter;
	SearchInfo m_SearchInfo;
	TT m_TT;
	int m_RootPly;
	Move m_PV[MAX_PLY + 1];
	int m_History[2][4096];
	SearchPly m_Plies[MAX_PLY + 1];
};

inline const SearchInfo& Search::info() const
{
	return m_SearchInfo;
}
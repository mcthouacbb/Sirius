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

class Search
{
public:
	static constexpr uint32_t TIME_CHECK_INTERVAL = 2048;

	Search(Board& board);

	int iterDeep(int maxDepth, int& depthSearched);

	int search(int depth, SearchPly* searchPly, int alpha, int beta, bool isPV);
	int qsearch(int alpha, int beta);

	const SearchInfo& info() const;

	void setTime(Duration clock, Duration inc);
private:
	void reset();
	void storeKiller(SearchPly* ply, Move killer);

	Board& m_Board;
	TimeManager m_TimeMan;
	bool m_ShouldStop;
	uint32_t m_TimeCheckCounter;
	SearchInfo m_SearchInfo;
	TT m_TT;
	int m_RootPly;
	Move m_PV[MAX_PLY];
	int m_History[2][4096];
	SearchPly m_Plies[MAX_PLY];
};

inline const SearchInfo& Search::info() const
{
	return m_SearchInfo;
}

inline void Search::setTime(Duration clock, Duration inc)
{
	m_TimeMan.setTimeLeft(clock, inc);
}
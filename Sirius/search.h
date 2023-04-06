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

class Search
{
public:
	static constexpr uint32_t TIME_CHECK_INTERVAL = 2048;

	Search(Board& board);

	int iterDeep(int maxDepth, int& depthSearched);

	int search(int depth, SearchPly* searchPly, int alpha, int beta, bool isPV);
	int qsearch(int alpha, int beta);

	const Move* pvBegin() const;
	const Move* pvEnd() const;

	void setTime(Duration clock, Duration inc);
private:
	void reset();
	void storeKiller(SearchPly* ply, Move killer);

	Board& m_Board;
	TimeManager m_TimeMan;
	bool m_ShouldStop;
	uint32_t m_TimeCheckCounter;
	TT m_TT;
	int m_RootPly;
	uint64_t m_Nodes;
	uint64_t m_QNodes;
	uint64_t m_TTMoves;
	uint64_t m_TTEvals;
	uint32_t m_PVLength;
	Move m_PV[MAX_PLY];
	int m_History[2][4096];
	SearchPly m_Plies[MAX_PLY];
};

inline void Search::setTime(Duration clock, Duration inc)
{
	m_TimeMan.setTimeLeft(clock, inc);
}

inline const Move* Search::pvBegin() const
{
	return m_PV;
}

inline const Move* Search::pvEnd() const
{
	return m_PV + m_PVLength;
}
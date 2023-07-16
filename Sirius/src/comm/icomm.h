#pragma once

#include "../board.h"
#include "../search.h"
#include "../time_man.h"
#include <deque>
#include <vector>
#include <string>

namespace comm
{

struct SearchInfo
{
	int depth;
	Duration time;
	const Move* pvBegin, *pvEnd;
	int score;
};

class IComm
{
public:
	IComm();
	~IComm() = default;

	void setToFen(const char* fen);
	void makeMove(Move move);
	void unmakeMove();

	virtual void reportSearchInfo(const SearchInfo& info) const = 0;
	virtual void execCommand(const std::string& command) = 0;
private:
	void calcLegalMoves();
protected:
	Board m_Board;
	std::deque<BoardState> m_PrevStates;
	std::vector<Move> m_PrevMoves;
	Move m_LegalMoves[256];
	uint32_t m_MoveCount;
	Search m_Search;
};

extern IComm* currComm;

}
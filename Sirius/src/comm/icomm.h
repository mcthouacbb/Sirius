#pragma once

#include "../board.h"
#include "../search.h"
#include "../time_man.h"
#include <deque>
#include <vector>
#include <string>

namespace comm
{

class IComm
{
public:
	IComm();
	virtual ~IComm() = default;

	void setToFen(const char* fen);
	void makeMove(Move move);
	void unmakeMove();

	virtual void run() = 0;
	virtual void reportSearchInfo(const SearchInfo& info) const = 0;
	virtual void reportBestMove(Move move) const = 0;
private:
	void calcLegalMoves();
protected:
	std::unique_lock<std::mutex> lockStdout() const;

	Board m_Board;
	mutable std::mutex m_StdoutMutex;
	std::deque<BoardState> m_PrevStates;
	std::vector<Move> m_PrevMoves;
	Move m_LegalMoves[256];
	uint32_t m_MoveCount;
	search::Search m_Search;
};

extern IComm* currComm;

}
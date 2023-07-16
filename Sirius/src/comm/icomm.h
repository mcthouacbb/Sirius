#pragma once

#include "../board.h"
#include "../search.h"
#include <deque>
#include <vector>
#include <string>

namespace comm
{

class IComm
{
public:
	IComm();
	~IComm() = default;

	void setToFen(const char* fen);
	void makeMove(Move move);
	void unmakeMove();

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

}
#pragma once

#include "../board.h"
#include <deque>
#include <vector>
#include <string>

namespace comm
{

class IComm
{
public:
	IComm() = default;
	~IComm() = default;

	void setFen(const char* fen);
	void makeMove(Move move);
	void unmakeMove(Move move);

	virtual void execCommand(const std::string& command) = 0;
private:
	void calcLegalMoves();

	Board m_Board;
	std::deque<BoardState> m_PrevStates;
	std::vector<Move> m_PrevMoves;
	Move m_LegalMoves[256];
	uint32_t m_MoveCount;
};

}
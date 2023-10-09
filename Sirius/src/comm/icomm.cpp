#include "icomm.h"
#include "../movegen.h"

namespace comm
{
    
IComm* currComm;

IComm::IComm()
	: m_BoardStates(1, BoardState{}), m_Board(m_BoardStates.front()), m_Search(m_Board)
{
	calcLegalMoves();
}

void IComm::setToFen(const char* fen)
{
	m_BoardStates.resize(1);
	m_PrevMoves.clear();

	m_Board.setToFen(fen);
	calcLegalMoves();
}

void IComm::makeMove(Move move)
{
	m_BoardStates.push_back({});
	m_PrevMoves.push_back(move);

	m_Board.makeMove(move, m_BoardStates.back());
	calcLegalMoves();
}

void IComm::unmakeMove()
{
	m_Board.unmakeMove(m_PrevMoves.back());

	m_BoardStates.pop_back();
	m_PrevMoves.pop_back();
	calcLegalMoves();
}

void IComm::calcLegalMoves()
{
	Move* end = genMoves<MoveGenType::LEGAL>(m_Board, m_LegalMoves);
	m_MoveCount = static_cast<uint32_t>(end - m_LegalMoves);
}

std::unique_lock<std::mutex> IComm::lockStdout() const
{
	return std::unique_lock<std::mutex>(m_StdoutMutex);
}


}

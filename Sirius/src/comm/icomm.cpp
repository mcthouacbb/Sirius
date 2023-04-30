#include "icomm.h"
#include "../movegen.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#elif
#include <unistd.h>
#endif

namespace comm
{

IComm::IComm()
	: m_Search(m_Board), m_State(CommState::IDLE)
{
	calcLegalMoves();
}

void IComm::setToFen(const char* fen)
{
	m_PrevStates.clear();
	m_PrevMoves.clear();

	m_Board.setToFen(fen);
	calcLegalMoves();
}

void IComm::makeMove(Move move)
{
	m_PrevStates.push_back({});
	m_PrevMoves.push_back(move);

	m_Board.makeMove(move, m_PrevStates.back());
	calcLegalMoves();
}

void IComm::unmakeMove()
{
	m_Board.unmakeMove(m_PrevMoves.back());

	m_PrevStates.pop_back();
	m_PrevMoves.pop_back();
	calcLegalMoves();
}

void IComm::calcLegalMoves()
{
	Move* end = genMoves<MoveGenType::LEGAL>(m_Board, m_LegalMoves);
	m_MoveCount = end - m_LegalMoves;
}


}
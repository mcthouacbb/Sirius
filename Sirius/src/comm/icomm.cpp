#include "icomm.h"
#include "../movegen.h"

namespace comm
{

IComm::IComm()
	: m_State(CommState::IDLE), m_Search(m_Board)
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
	m_MoveCount = static_cast<uint32_t>(end - m_LegalMoves);
}


}
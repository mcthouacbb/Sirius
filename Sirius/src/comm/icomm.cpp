#include "icomm.h"

namespace comm
{

IComm* currComm;

IComm::IComm()
    : m_Board(), m_Search(m_Board)
{
    calcLegalMoves();
}

void IComm::setToFen(const char* fen, bool frc)
{
    m_PrevMoves.clear();

    m_Board.setToFen(fen, frc);
    calcLegalMoves();
}

void IComm::makeMove(Move move)
{
    m_PrevMoves.push_back(move);

    m_Board.makeMove(move);
    calcLegalMoves();
}

void IComm::unmakeMove()
{
    m_Board.unmakeMove();

    m_PrevMoves.pop_back();
    calcLegalMoves();
}

void IComm::calcLegalMoves()
{
    m_LegalMoves.clear();
    genMoves<MoveGenType::LEGAL>(m_Board, m_LegalMoves);
}

std::unique_lock<std::mutex> IComm::lockStdout() const
{
    return std::unique_lock<std::mutex>(m_StdoutMutex);
}


}

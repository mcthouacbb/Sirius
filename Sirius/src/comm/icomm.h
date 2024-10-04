#pragma once

#include "../board.h"
#include "../search.h"
#include "../time_man.h"
#include "../movegen.h"
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

    virtual void reportSearchInfo(const SearchInfo& info) const = 0;
    virtual void reportBestMove(Move bestMove) const = 0;
private:
    void calcLegalMoves();
protected:
    std::unique_lock<std::mutex> lockStdout() const;

    mutable std::mutex m_StdoutMutex;
    std::vector<Move> m_PrevMoves;
    Board m_Board;
    MoveList m_LegalMoves;
    search::Search m_Search;
};

extern IComm* currComm;

}

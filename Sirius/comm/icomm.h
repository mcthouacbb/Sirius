#include "../board.h"
#include <deque>

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
private:
	Board m_Board;
	std::deque<Move> m_PrevMoves;
	Move m_LegalMoves[256];
	uint32_t m_MoveCount;
};

}
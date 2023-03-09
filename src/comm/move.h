#include "../board.h"

namespace comm
{

struct MoveStrFind
{
	Move* move;
	const char* end;
};

MoveStrFind findMoveFromPCN(Move* begin, Move* end, const char* moveStr);
MoveStrFind findMoveFromSAN(const Board& board, Move* begin, Move* end, const char* moveStr);

std::string convMoveToPCN(Move move);
std::string convMoveToSAN(const Board& board, Move* begin, Move* end, Move move);

}
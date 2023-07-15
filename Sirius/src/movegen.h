#include "move.h"
#include "board.h"

enum class MoveGenType
{
	LEGAL
};

struct CheckInfo
{
	BitBoard checkBB;
	BitBoard moveMask;
	BitBoard checkers;
	BitBoard pinned;
};

template<MoveGenType type>
Move* genMoves(const Board& board, Move* moves);
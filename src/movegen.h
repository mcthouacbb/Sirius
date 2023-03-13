#include "board.h"

enum class MoveGenType
{
	LEGAL,
	CAPTURES
};

struct CheckInfo
{
	BitBoard checkBB;
	BitBoard moveMask;
	BitBoard checkers;
	BitBoard pinned;
};

CheckInfo calcCheckInfo(const Board& board, Color color);

template<MoveGenType type>
Move* genMoves(const Board& board, Move* moves, const CheckInfo& checkInfo);
#include "../board.h"

namespace eval::endgames
{

using EndgameFunc = int(const Board& board, Color strongSide);

struct Endgame
{
	explicit Endgame(Color c, EndgameFunc* func)
		: strongSide(c), func(func)
	{

	}

	int operator()(const Board& board)
	{
		int result = (*func)(board, strongSide);
		return strongSide == board.sideToMove() ? result : -result;
	}

	EndgameFunc* func;
	Color strongSide;
};

void init();
Endgame* probe(const Board& board);

}

#pragma once

#include "board.h"

class MoveOrdering
{
public:
	MoveOrdering(const Board& board, Move* begin, Move* end);
	MoveOrdering(const Board& board, Move* begin, Move* end, Move hashMove, Move (&killers)[2], int (&history)[4096]);

	Move selectMove(uint32_t index);
private:
	static constexpr int KILLER_BONUS = 10;
	static constexpr int CAPTURE_BONUS = 20;
	static constexpr int HISTORY_BONUS = -1000000;

	Move* m_Moves;
	uint32_t m_Size;
	int m_MoveScores[256];
};
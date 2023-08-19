#pragma once

#include "board.h"

inline int historyIndex(Move move)
{
	return move.fromTo();
}

struct ExtMove
{
	Move move;
	int score;
};

class MoveOrdering
{
public:
	static constexpr int HISTORY_MAX = 65536;
	static constexpr int KILLER_SCORE = 65550;
	static constexpr int PROMOTION_SCORE = 65555;
	static constexpr int CAPTURE_SCORE = 65560;

	MoveOrdering(const Board& board, Move* begin, Move* end);
	MoveOrdering(const Board& board, Move* begin, Move* end, Move hashMove, Move (&killers)[2], int (&history)[4096]);

	ExtMove selectMove(uint32_t index);
private:

	Move* m_Moves;
	uint32_t m_Size;
	int m_MoveScores[256];
};
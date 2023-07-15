#include "board.h"

class MoveOrdering
{
public:
	MoveOrdering(const Board& board, Move* begin, Move* end);

	Move selectMove(uint32_t index);
private:
	Move* m_Moves;
	uint32_t m_Size;
	int m_MoveScores[256];
};
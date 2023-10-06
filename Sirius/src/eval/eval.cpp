#include "eval.h"

namespace eval
{

int evaluate(const Board& board)
{
	if (!eval::canForceMate(board))
		return SCORE_DRAW;
	return rawEval(board);
}

int rawEval(const Board& board)
{
	Color color = board.sideToMove();
    PackedScore matPsqt = board.evalState().materialPsqt;
	return (color == Color::WHITE ? 1 : -1) * eval::getFullEval(matPsqt.mg(), matPsqt.eg(), board.evalState().phase);
}


}

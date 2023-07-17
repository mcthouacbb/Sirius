#include "eval.h"

namespace eval
{

int evalMaterialMG(const Board& board, Color color)
{
	return board.evalState().materialMG[static_cast<int>(color)];
}

int evalMaterialEG(const Board& board, Color color)
{
	return board.evalState().materialEG[static_cast<int>(color)];
}

int evaluate(const Board& board)
{
	Color color = board.currPlayer();
	Color opp = flip(color);
	int matMG = evalMaterialMG(board, color) - evalMaterialMG(board, opp);
	int matEG = evalMaterialEG(board, color) - evalMaterialEG(board, opp);

	return eval::getFullEval(matMG, matEG, board.evalState().phase);
}


}
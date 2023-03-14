#include "eval.h"

namespace eval
{

namespace
{

int evalMaterialMG(const Board& board, Color color)
{
	return board.evalState().materialMG[static_cast<int>(color)];
}

int evalMaterialEG(const Board& board, Color color)
{
	return board.evalState().materialEG[static_cast<int>(color)];
}

int evalPSQTMG(const Board& board, Color color)
{
	return board.evalState().psqtMG[static_cast<int>(color)];
}

int evalPSQTEG(const Board& board, Color color)
{
	return board.evalState().psqtEG[static_cast<int>(color)];
}

}

int evaluate(const Board& board)
{
	Color color = board.currPlayer();
	Color opp = flip(color);
	int matMG = evalMaterialMG(board, color) - evalMaterialMG(board, opp);
	int matEG = evalMaterialEG(board, color) - evalMaterialEG(board, opp);

	int psqtMG = evalPSQTMG(board, color) - evalPSQTMG(board, opp);
	int psqtEG = evalPSQTEG(board, color) - evalPSQTEG(board, opp);
	
	return eval::getFullEval(matMG + psqtMG, matEG + psqtEG, board.evalState().phase);
}


}
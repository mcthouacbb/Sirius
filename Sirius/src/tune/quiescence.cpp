#include "quiescence.h"
#include "../movegen.h"
#include "../eval/eval.h"
#include "../move_ordering.h"

namespace tune
{

int qsearch(Board& board, const EvalParams& params, const EvalCache& cache, int& nodes, int alpha, int beta)
{
	nodes++;
	if (eval::isImmediateDraw(board))
		return eval::DRAW;

	int score = evaluate(board, params, cache);

	if (score >= beta)
		return beta;
	if (score > alpha)
		alpha = score;

	Move captures[256];
	Move* end = genMoves<MoveGenType::CAPTURES>(board, captures);

	MoveOrdering ordering(board, captures, end);

	BoardState state;
	for (uint32_t i = 0; i < end - captures; i++)
	{
		Move move = ordering.selectMove(i);
		board.makeMove(move, state);
		int moveScore = -qsearch(board, params, cache, nodes, -beta, -alpha);
		board.unmakeMove(move);

		if (moveScore >= beta)
			return beta;

		if (moveScore > alpha)
			alpha = moveScore;
	}

	return alpha;
}


}
#include "search.h"
#include "eval/eval.h"
#include "movegen.h"
#include "move_ordering.h"
#include "comm/move.h"
#include "comm/icomm.h"
#include "search_params.h"
#include <cstring>
#include <climits>

namespace search
{

namespace
{

void updateHistory(int& history, int bonus)
{
	history -= history * std::abs(bonus) / MoveOrdering::HISTORY_MAX;
	history += bonus;
}


}

int lmrTable[64][64];

void init()
{
	for (int d = 1; d < 64; d++)
	{
		for (int i = 1; i < 64; i++)
		{
			lmrTable[d][i] = static_cast<int>(LMR_BASE + std::log(static_cast<double>(d)) * std::log(static_cast<double>(i)) / LMR_DIVISOR);
		}
	}
}

Search::Search(Board& board)
	: m_Board(board), m_TT(1024 * 1024), m_RootPly(0)
{

}

void Search::storeKiller(SearchPly* ply, Move killer)
{
	if (ply->killers[0] != killer)
	{
		ply->killers[1] = ply->killers[0];
		ply->killers[0] = killer;
	}
}

void Search::reset()
{
	memset(m_History, 0, sizeof(m_History));
	m_SearchInfo.nodes = 0;

	for (int i = 0; i <= MAX_PLY; i++)
	{
		m_Plies[i].killers[0] = m_Plies[i].killers[1] = Move();
		m_Plies[i].pv = nullptr;
		m_Plies[i].pvLength = 0;
	}

	m_TT.incAge();
}

void Search::newGame()
{
	reset();
	m_TT.reset();
}

int Search::search(int depth)
{
	Move pv[256];
	m_Plies[0].pv = pv;
	int searchScore = search(depth, m_Plies, -SCORE_MAX, SCORE_MAX, true);
	memcpy(m_PV, pv, m_Plies[0].pvLength * sizeof(Move));
	m_SearchInfo.depth = depth;
	m_SearchInfo.time = Duration(0);
	m_SearchInfo.pvBegin = m_PV;
	m_SearchInfo.pvEnd = m_PV + m_Plies[0].pvLength;
	m_SearchInfo.score = searchScore;
	return searchScore;
}

int Search::qsearch()
{
	return qsearch(m_Plies, -SCORE_MAX, SCORE_MAX);
}

int Search::iterDeep(const SearchLimits& limits, bool report)
{
	int maxDepth = std::min(limits.maxDepth, MAX_PLY - 1);
	Move pv[MAX_PLY + 1];
	int score = 0;

	reset();
	m_ShouldStop = false;
	m_CheckCounter = TIME_CHECK_INTERVAL;
	m_TimeMan.setLimits(limits, m_Board.sideToMove());
	m_TimeMan.startSearch();

	for (int depth = 1; depth <= maxDepth; depth++)
	{
		m_Plies[0].pv = pv;
		int searchScore = aspWindows(depth, score);
		if (m_ShouldStop)
			break;
		memcpy(m_PV, m_Plies[0].pv, m_Plies[0].pvLength * sizeof(Move));
		score = searchScore;
		m_SearchInfo.depth = depth;
		m_SearchInfo.time = m_TimeMan.elapsed();
		m_SearchInfo.pvBegin = m_PV;
		m_SearchInfo.pvEnd = m_PV + m_Plies[0].pvLength;
		m_SearchInfo.score = searchScore;
		if (report)
			comm::currComm->reportSearchInfo(m_SearchInfo);
	}
	return score;
}

int Search::aspWindows(int depth, int prevScore)
{
	int delta = ASP_INIT_DELTA;
	int alpha = prevScore - delta;
	int beta = prevScore + delta;

	while (true)
	{
		int searchScore = search(depth, &m_Plies[0], alpha, beta, true);
		if (m_ShouldStop)
			return searchScore;

		if (searchScore <= alpha)
		{
			beta = (alpha + beta) / 2;
			alpha -= delta;
		}
		else if (searchScore >= beta)
			beta += delta;
		else
			return searchScore;
		delta *= 2;
	}
}

BenchData Search::benchSearch(int depth)
{
	SearchLimits limits;
	limits.policy = SearchPolicy::INFINITE;
	limits.maxDepth = depth;

	iterDeep(limits, false);

	BenchData data;
	data.nodes = m_SearchInfo.nodes;

	return data;
}

int Search::search(int depth, SearchPly* searchPly, int alpha, int beta, bool isPV)
{
	if (--m_CheckCounter == 0)
	{
		m_CheckCounter = TIME_CHECK_INTERVAL;
		if (m_TimeMan.shouldStop(m_SearchInfo))
		{
			m_ShouldStop = true;
			return alpha;
		}

		if (comm::currComm->checkInput())
		{
			m_ShouldStop = true;
			return alpha;
		}
	}

	m_SearchInfo.nodes++;

	alpha = std::max(alpha, -SCORE_MATE + m_RootPly);
	beta = std::min(beta, SCORE_MATE - m_RootPly);
	if (alpha >= beta)
		return alpha;

	bool root = m_RootPly == 0;

	if (eval::isImmediateDraw(m_Board) || m_Board.isDraw(m_RootPly))
	{
		searchPly->pvLength = 0;
		return SCORE_DRAW;
	}

	if (m_RootPly >= MAX_PLY)
	{
		searchPly->pvLength = 0;
		return eval::evaluate(m_Board);
	}

	if (depth <= 0)
		return qsearch(searchPly, alpha, beta);

	int hashScore = INT_MIN;
	Move hashMove = Move();
	TTBucket* bucket = m_TT.probe(m_Board.zkey(), depth, m_RootPly, alpha, beta, hashScore, hashMove);

	if (hashScore != INT_MIN && !isPV)
	{
		searchPly->pvLength = 0;
		return hashScore;
	}

	int staticEval = eval::evaluate(m_Board);
	BoardState state;

	if (!isPV && !m_Board.checkers())
	{
		// reverse futility pruning
		if (depth <= RFP_MAX_DEPTH && staticEval >= beta + RFP_MARGIN * depth)
		{
			searchPly->pvLength = 0;
			return staticEval;
		}

		// null move pruning

		if (m_Board.pliesFromNull() > 0)
		{
			BitBoard nonPawns = m_Board.getColor(m_Board.sideToMove()) ^ m_Board.getPieces(m_Board.sideToMove(), PieceType::PAWN);
			if ((nonPawns & (nonPawns - 1)) && depth >= NMP_MIN_DEPTH)
			{
				int r = NMP_BASE_REDUCTION;
				m_Board.makeNullMove(state);
				m_RootPly++;
				int nullScore = -search(depth - r, searchPly + 1, -beta, -beta + 1, false);
				m_RootPly--;
				m_Board.unmakeNullMove();
				if (nullScore >= beta)
				{
					searchPly->pvLength = 0;
					return nullScore;
				}
			}
		}
	}

	Move moves[256];
	Move* end = genMoves<MoveGenType::LEGAL>(m_Board, moves);

	if (moves == end)
	{
		searchPly->pvLength = 0;
		if (m_Board.checkers())
			return -SCORE_MATE + m_RootPly;
		return SCORE_DRAW;
	}
	MoveOrdering ordering(
		m_Board,
		moves,
		end,
		hashMove,
		searchPly->killers,
		m_History[static_cast<int>(m_Board.sideToMove())]
	);

	Move childPV[MAX_PLY + 1];
	searchPly[1].pv = childPV;

	searchPly->bestMove = Move();

	TTEntry::Type type = TTEntry::Type::UPPER_BOUND;
	bool inCheck = m_Board.checkers() != 0;

	Move quietsTried[256];
	int numQuietsTried = 0;

	int bestScore = -SCORE_MAX;

	for (uint32_t i = 0; i < end - moves; i++)
	{
		auto [move, moveScore] = ordering.selectMove(i);
		bool givesCheck = m_Board.givesCheck(move);
		bool isCapture = m_Board.getPieceAt(move.dstPos()) != PIECE_NONE;
		bool isPromotion = move.type() == MoveType::PROMOTION;
		bool quietLosing = moveScore < MoveOrdering::KILLER_SCORE;

		int baseLMR = lmrTable[std::min(depth, 63)][std::min(i, 63u)];

		if (!root && quietLosing && bestScore > -SCORE_WIN)
		{
			int lmrDepth = std::max(depth - baseLMR, 0);
			if (lmrDepth <= FP_MAX_DEPTH &&
				!inCheck &&
				alpha < SCORE_WIN &&
				staticEval + FP_BASE_MARGIN + FP_DEPTH_MARGIN * lmrDepth <= alpha)
			{
				continue;
			}

			if (!isPV &&
				depth <= MAX_SEE_PRUNE_DEPTH &&
				!m_Board.see_margin(move, depth * SEE_PRUNE_MARGIN))
				continue;
		}

		int reduction = 0;
		if (i >= (isPV ? LMR_MIN_MOVES_PV : LMR_MIN_MOVES_NON_PV) &&
			depth >= LMR_MIN_DEPTH &&
			quietLosing &&
			!inCheck)
		{
			reduction = baseLMR;

			reduction -= isPV;
			reduction -= givesCheck;

			reduction = std::clamp(reduction, 0, depth - 2);
		}
		m_Board.makeMove(move, state);
		if (!isPromotion && !isCapture)
			quietsTried[numQuietsTried++] = move;
		m_RootPly++;

		int newDepth = depth + givesCheck - 1;
		int score;
		if (i == 0)
			score = -search(newDepth, searchPly + 1, -beta, -alpha, isPV);
		else
		{
			score = -search(newDepth - reduction, searchPly + 1, -(alpha + 1), -alpha, false);

			/*if (moveScore > alpha && reduction)
				moveScore = -search(newDepth, searchPly + 1, -(alpha + 1), -alpha, false);*/

			if (score > alpha && (isPV || reduction > 0))
				score = -search(newDepth, searchPly + 1, -beta, -alpha, true);
		}
		m_RootPly--;
		m_Board.unmakeMove(move);

		if (m_ShouldStop)
			return alpha;

		if (score > bestScore)
		{
			bestScore = score;

			if (bestScore >= beta)
			{
				if (!isPromotion && !isCapture)
				{
					storeKiller(searchPly, move);

					// formula from akimbo
					int historyBonus = std::min(16 * depth * depth, 1200);
					updateHistory(m_History[static_cast<int>(m_Board.sideToMove())][historyIndex(move)], historyBonus);
					for (int j = 0; j < numQuietsTried - 1; j++)
					{
						updateHistory(m_History[static_cast<int>(m_Board.sideToMove())][historyIndex(quietsTried[j])], -historyBonus);
					}
				}
				m_TT.store(bucket, m_Board.zkey(), depth, m_RootPly, bestScore, move, TTEntry::Type::LOWER_BOUND);
				return bestScore;
			}

			if (bestScore > alpha)
			{
				type = TTEntry::Type::EXACT;
				alpha = bestScore;
				searchPly->bestMove = move;
				if (isPV)
				{
					searchPly->pv[0] = move;
					searchPly->pvLength = searchPly[1].pvLength + 1;
					memcpy(searchPly->pv + 1, searchPly[1].pv, searchPly[1].pvLength * sizeof(Move));
				}
			}
		}
	}

	m_TT.store(bucket, m_Board.zkey(), depth, m_RootPly, bestScore, searchPly->bestMove, type);

	return bestScore;
}

int Search::qsearch(SearchPly* searchPly, int alpha, int beta)
{
	searchPly->pvLength = 0;
	if (eval::isImmediateDraw(m_Board))
		return SCORE_DRAW;

	int hashScore = INT_MIN;
	Move hashMove = Move();
	// qsearch is always depth 0
	m_TT.probe(m_Board.zkey(), 0, m_RootPly, alpha, beta, hashScore, hashMove);

	if (hashScore != INT_MIN)
	{
		searchPly->pvLength = 0;
		return hashScore;
	}

	int eval = eval::evaluate(m_Board);

	m_SearchInfo.nodes++;

	if (eval >= beta)
		return eval;
	if (eval > alpha)
		alpha = eval;

	if (m_RootPly >= MAX_PLY)
		return alpha;

	Move childPV[MAX_PLY + 1];
	searchPly[1].pv = childPV;

	Move captures[256];
	Move* end = genMoves<MoveGenType::CAPTURES>(m_Board, captures);

	MoveOrdering ordering(m_Board, captures, end, hashMove);

	BoardState state;
	for (uint32_t i = 0; i < end - captures; i++)
	{
		auto [move, moveScore] = ordering.selectMove(i);
		if (!m_Board.see_margin(move, 0))
			continue;
		m_Board.makeMove(move, state);
		m_RootPly++;
		int score = -qsearch(searchPly + 1, -beta, -alpha);
		m_Board.unmakeMove(move);
		m_RootPly--;

		if (score > eval)
		{
			eval = score;

			if (eval >= beta)
				return eval;

			if (eval > alpha)
			{
				searchPly->pvLength = searchPly[1].pvLength + 1;
				searchPly->pv[0] = move;

				memcpy(searchPly->pv + 1, searchPly[1].pv, searchPly[1].pvLength * sizeof(Move));

				alpha = eval;
			}
		}
	}

	return eval;
}


}

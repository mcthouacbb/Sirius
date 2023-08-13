#include "search.h"
#include "eval/eval.h"
#include "movegen.h"
#include "move_ordering.h"
#include "comm/move.h"
#include "comm/icomm.h"
#include <cstring>
#include <climits>

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

int Search::search(int depth)
{
	Move pv[256];
	m_Plies[0].pv = pv;
	int searchScore = search(depth, m_Plies, eval::NEG_INF, eval::POS_INF, true);
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
	return qsearch(m_Plies, eval::NEG_INF, eval::POS_INF);
}

int Search::iterDeep(const SearchLimits& limits)
{
	int maxDepth = std::min(limits.maxDepth, MAX_PLY - 1);
	Move pv[MAX_PLY + 1];
	int score = 0;

	reset();
	m_ShouldStop = false;
	m_CheckCounter = CHECK_INTERVAL;
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
		comm::currComm->reportSearchInfo(m_SearchInfo);
	}
	return score;
}

int Search::aspWindows(int depth, int prevScore)
{
	int delta = 25;
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

int Search::search(int depth, SearchPly* searchPly, int alpha, int beta, bool isPV)
{
	if (--m_CheckCounter == 0)
	{
		m_CheckCounter = CHECK_INTERVAL;
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

	alpha = std::max(alpha, eval::CHECKMATE + m_RootPly);
	beta = std::min(beta, -eval::CHECKMATE - m_RootPly);
	if (alpha >= beta)
		return alpha;

	if (eval::isImmediateDraw(m_Board) || m_Board.isDraw(m_RootPly))
	{
		searchPly->pvLength = 0;
		return eval::DRAW;
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
		if (depth <= 8 && staticEval >= beta + 75 * depth)
		{
			searchPly->pvLength = 0;
			return staticEval;
		}

		// null move pruning

		if (m_Board.pliesFromNull() > 0)
		{
			int R = 2;
			BitBoard nonPawns = m_Board.getColor(m_Board.sideToMove()) ^ m_Board.getPieces(m_Board.sideToMove(), PieceType::PAWN);
			if ((nonPawns & (nonPawns - 1)) && depth >= R)
			{
				m_Board.makeNullMove(state);
				m_RootPly++;
				int nullScore = -search(depth - R - 1, searchPly + 1, -beta, -beta + 1, false);
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
			return eval::CHECKMATE + m_RootPly;
		return eval::STALEMATE;
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

	bool fprune =
		depth == 1 &&
		staticEval + FUTILITY_MARGIN < alpha &&
		!eval::isMateScore(alpha) &&
		!eval::isMateScore(beta) &&
		!inCheck;

	int bestScore = eval::NEG_INF;

	for (uint32_t i = 0; i < end - moves; i++)
	{
		Move move = ordering.selectMove(i);
		bool givesCheck = m_Board.givesCheck(move);
		bool isCapture = m_Board.getPieceAt(move.dstPos()) != PIECE_NONE;
		bool isPromotion = move.type() == MoveType::PROMOTION;

		if (fprune &&
			!givesCheck &&
			!isCapture &&
			!isPromotion &&
			i > 3)
		{
			continue;
		}

		int reduction = 0;
		if (i >= (isPV ? 15u : 4u) &&
			depth >= 3 &&
			!givesCheck &&
			!isCapture &&
			!isPromotion &&
			!inCheck)
		{
			reduction = 1;
		}
		m_Board.makeMove(move, state);
		m_RootPly++;

		int newDepth = depth + givesCheck - 1;
		int moveScore;
		if (i == 0)
			moveScore = -search(newDepth, searchPly + 1, -beta, -alpha, isPV);
		else
		{
			moveScore = -search(newDepth - reduction, searchPly + 1, -(alpha + 1), -alpha, false);

			/*if (moveScore > alpha && reduction)
				moveScore = -search(newDepth, searchPly + 1, -(alpha + 1), -alpha, false);*/

			if (moveScore > alpha && (isPV || reduction > 0))
				moveScore = -search(newDepth, searchPly + 1, -beta, -alpha, true);
		}
		m_RootPly--;
		m_Board.unmakeMove(move);

		if (m_ShouldStop)
			return alpha;

		if (moveScore > bestScore)
		{
			bestScore = moveScore;

			if (bestScore >= beta)
			{
				if (move.type() != MoveType::PROMOTION && state.capturedPiece == PIECE_NONE)
				{
					storeKiller(searchPly, move);
					m_History[static_cast<int>(m_Board.sideToMove())][move.fromTo()] += depth * depth;
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
		return eval::DRAW;

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

	MoveOrdering ordering(m_Board, captures, end);

	BoardState state;
	for (uint32_t i = 0; i < end - captures; i++)
	{
		Move move = ordering.selectMove(i);
		if (!m_Board.see_margin(move, 0))
			continue;
		m_Board.makeMove(move, state);
		m_RootPly++;
		int moveScore = -qsearch(searchPly + 1, -beta, -alpha);
		m_Board.unmakeMove(move);
		m_RootPly--;

		if (moveScore > eval)
		{
			eval = moveScore;

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

#include "search.h"
#include "eval/eval.h"
#include "movegen.h"
#include "move_ordering.h"
#include "comm/move.h"
#include <cstring>
#include <climits>

Search::Search(Board& board)
	: m_Board(board), m_RootPly(0), m_TT(1024 * 1024)
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

	for (int i = 0; i < MAX_PLY; i++)
	{
		m_Plies[i].killers[0] = m_Plies[i].killers[1] = Move();
		m_Plies[i].pv = nullptr;
		m_Plies[i].pvLength = 0;
	}

	m_TT.incAge();
}

int Search::iterDeep(int maxDepth)
{
	int score;
	reset();
	for (int depth = 1; depth <= maxDepth; depth++)
	{
		m_Nodes = 0;
		m_QNodes = 0;
		m_TTEvals = 0;
		m_TTMoves = 0;
		m_Plies[0].pv = m_PV;
		int searchScore = search(depth, &m_Plies[0], eval::NEG_INF, eval::POS_INF, true);
		score = searchScore;
		std::cout << "Depth: " << depth << std::endl;
		std::cout << "\tNodes: " << m_Nodes << std::endl;
		std::cout << "\tQNodes: " << m_QNodes << std::endl;
		std::cout << "\tTT Evals: " << m_TTEvals << std::endl;
		std::cout << "\tTT Moves: " << m_TTMoves << std::endl;
		std::cout << "\tPV Length: " << m_Plies[0].pvLength << std::endl;
		std::cout << "\tEval: " << searchScore << std::endl;
		std::cout << "\tPV: ";
		for (int i = 0; i < m_Plies[0].pvLength; i++)
		{
			std::cout << comm::convMoveToPCN(m_PV[i]) << ' ';
		}
		std::cout << std::endl;

		if (eval::isMateScore(searchScore))
			return score;
	}
	return score;
}


int Search::search(int depth, SearchPly* searchPly, int alpha, int beta, bool isPV)
{
	alpha = std::max(alpha, eval::CHECKMATE + m_RootPly);
	beta = std::min(beta, -eval::CHECKMATE - m_RootPly);
	if (alpha >= beta)
		return alpha;

	if (eval::isImmediateDraw(m_Board) || m_Board.halfMoveClock() >= 100)
	{
		searchPly->pvLength = 0;
		return eval::DRAW;
	}
	if (m_Board.reversiblePly() >= 4)
	{
		int repetitions = m_Board.repetitions();
		if (repetitions == 2 || (repetitions == 1 && m_RootPly >= 2))
		{
			searchPly->pvLength = 0;
			return eval::DRAW;
		}
	}

	if (depth <= 0)
	{
		searchPly->pvLength = 0;
		return qsearch(alpha, beta);
	}
	
	int hashScore = INT_MIN;
	Move hashMove = Move();
	TTBucket* bucket = m_TT.probe(m_Board.zkey(), depth, m_RootPly, alpha, beta, hashScore, hashMove);

	if (hashScore != INT_MIN && m_RootPly > 0)
	{
		searchPly->pvLength = 0;
		m_TTEvals++;
		return hashScore;
	}

	m_Nodes++;

	// CheckInfo checkInfo = calcCheckInfo(m_Board, m_Board.sideToMove());
	Move moves[256];
	Move* end = genMoves<MoveGenType::LEGAL>(m_Board, moves);

	if (moves == end)
	{
		searchPly->pvLength = 0;
		if (m_Board.checkers())
			return eval::CHECKMATE + m_RootPly;
		return eval::STALEMATE;
	}
	if (hashMove != Move())
		m_TTMoves++;
	MoveOrdering ordering(
		m_Board,
		moves,
		end,
		hashMove,
		searchPly->killers,
		m_History[static_cast<int>(m_Board.sideToMove())]
	);
	
	BoardState state;

	Move childPV[MAX_PLY];
	searchPly[1].pv = childPV;

	searchPly->bestMove = Move();

	TTEntry::Type type = TTEntry::Type::UPPER_BOUND;
	
	for (uint32_t i = 0; i < end - moves; i++)
	{
		Move move = ordering.selectMove(i);
		int extension = m_Board.givesCheck(move);
		m_Board.makeMove(move, state);
		m_RootPly++;
		int newDepth = depth + extension - 1;
		int moveScore;
		if (searchPly->bestMove == Move())
			moveScore = -search(newDepth, searchPly + 1, -beta, -alpha, isPV);
		else
		{
			moveScore = -search(newDepth, searchPly + 1, -(alpha + 1), -alpha, false);
			if (moveScore > alpha && isPV)
				moveScore = -search(newDepth, searchPly + 1, -beta, -alpha, true);
		}
		m_RootPly--;
		m_Board.unmakeMove(move);

		if (moveScore >= beta)
		{
			if (move.type() != MoveType::PROMOTION && !state.capturedPiece)
			{
				storeKiller(searchPly, move);
				m_History[static_cast<int>(m_Board.sideToMove())][move.fromTo()] += depth * depth;
			}
			m_TT.store(bucket, m_Board.zkey(), depth, m_RootPly, beta, move, TTEntry::Type::LOWER_BOUND);
			return beta;
		}

		if (moveScore > alpha)
		{
			type = TTEntry::Type::EXACT;
			alpha = moveScore;
			searchPly->bestMove = move;
			searchPly->pv[0] = move;
			searchPly->pvLength = searchPly[1].pvLength + 1;
			memcpy(searchPly->pv + 1, searchPly[1].pv, searchPly[1].pvLength * sizeof(Move));
		}
	}

	m_TT.store(bucket, m_Board.zkey(), depth, m_RootPly, alpha, searchPly->bestMove, type);

	return alpha;
}

int Search::qsearch(int alpha, int beta)
{
	if (eval::isImmediateDraw(m_Board))
		return eval::DRAW;

	int score = eval::evaluate(m_Board);

	m_QNodes++;
	if (score >= beta)
		return beta;
	if (score > alpha)
		alpha = score;

	// CheckInfo checkInfo = calcCheckInfo(m_Board, m_Board.sideToMove());

	Move captures[256];
	Move* end = genMoves<MoveGenType::CAPTURES>(m_Board, captures);

	MoveOrdering ordering(m_Board, captures, end);
	
	BoardState state;
	for (uint32_t i = 0; i < end - captures; i++)
	{
		Move move = ordering.selectMove(i);
		m_Board.makeMove(move, state);
		int moveScore = -qsearch(-beta, -alpha);
		m_Board.unmakeMove(move);

		if (moveScore >= beta)
			return beta;

		if (moveScore > alpha)
			alpha = moveScore;
	}

	return alpha;
}
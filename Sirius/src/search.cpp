#include "search.h"
#include "eval/eval.h"
#include "movegen.h"
#include "move_ordering.h"
#include "comm/move.h"
#include <cstring>

Search::Search(Board& board)
	: m_Board(board), m_RootPly(0)
{
	
}


int Search::iterDeep(int maxDepth)
{
	int score = 0;
	reset();
	m_ShouldStop = false;
	m_TimeCheckCounter = TIME_CHECK_INTERVAL;
	// 3 | 1
	m_TimeMan.setTimeLeft(Duration(180000000), Duration(1000000));
	m_TimeMan.startSearch();
	for (int depth = 1; depth <= maxDepth; depth++)
	{
		m_Nodes = 0;
		m_QNodes = 0;
		m_Plies[0].pv = m_PV;
		int searchScore = search(depth, eval::NEG_INF, eval::POS_INF);
		if (m_ShouldStop)
		{
			std::cout << "Depth: " << depth << std::endl;
			std::cout << "Time Ran Out" << std::endl;
			std::cout << std::endl;
			break;
		}
		score = searchScore;
		std::cout << "Depth: " << depth << std::endl;
		std::cout << "\tNodes: " << m_Nodes << std::endl;
		std::cout << "\tQNodes: " << m_QNodes << std::endl;
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

void printBoard(const Board& board);

int Search::search(int depth, int alpha, int beta)
{
	if (--m_TimeCheckCounter == 0)
	{
		m_TimeCheckCounter = TIME_CHECK_INTERVAL;
		if (m_TimeMan.shouldStop())
		{
			m_ShouldStop = true;
			return alpha;
		}
	}

	alpha = std::max(alpha, eval::CHECKMATE + m_RootPly);
	beta = std::min(beta, -eval::CHECKMATE - m_RootPly);
	if (alpha >= beta)
		return alpha;
	
	CheckInfo checkInfo = calcCheckInfo(m_Board, m_Board.currPlayer());

	if (checkInfo.checkers)
	{
		depth = std::max(depth + 1, 1);
	}
	
	if (depth <= 0)
	{
		m_Plies[m_RootPly].pvLength = 0;
		return qsearch(alpha, beta, 1);
	}

	m_Nodes++;


	Move moves[256];
	Move* end = genMoves<MoveGenType::LEGAL>(m_Board, moves, checkInfo);

	if (moves == end)
	{
		m_Plies[m_RootPly].pvLength = 0;
		if (checkInfo.checkers)
			return eval::CHECKMATE + m_RootPly;
		return eval::STALEMATE;
	}
	
	MoveOrdering ordering(m_Board, moves, end);
	
	BoardState state;

	Move childPV[MAX_PLY];
	m_Plies[m_RootPly + 1].pv = childPV;

	for (uint32_t i = 0; i < end - moves; i++)
	{
		Move move = ordering.selectMove(i);
		// for (int i = 0; i < 2 - depth; i++)
			// std::cout << '\t';
		// std::cout << "Depth: " << depth << ", " << it - moves << std::endl;
		// const auto& move = *it;
		m_Board.makeMove(move, state);
		m_RootPly++;
		int moveScore = -search(depth - 1, -beta, -alpha);
		m_RootPly--;
		m_Board.unmakeMove(move, state);

		if (m_ShouldStop)
			return alpha;

		if (moveScore >= beta)
		{
			return beta;
		}

		if (moveScore > alpha)
		{
			alpha = moveScore;

			m_Plies[m_RootPly].pv[0] = move;
			m_Plies[m_RootPly].pvLength = m_Plies[m_RootPly + 1].pvLength + 1;
			memcpy(m_Plies[m_RootPly].pv + 1, m_Plies[m_RootPly + 1].pv, m_Plies[m_RootPly + 1].pvLength * sizeof(Move));
		}
	}

	return alpha;
}

int Search::qsearch(int alpha, int beta, int depth)
{
	int score = eval::evaluate(m_Board);

	m_QNodes++;
	if (score >= beta)
		return beta;
	if (score > alpha)
		alpha = score;

	if (depth <= 0)
		return alpha;

	CheckInfo checkInfo = calcCheckInfo(m_Board, m_Board.currPlayer());

	Move captures[256];
	Move* end = genMoves<MoveGenType::CAPTURES>(m_Board, captures, checkInfo);

	MoveOrdering ordering(m_Board, captures, end);
	
	BoardState state;
	for (uint32_t i = 0; i < end - captures; i++)
	{
		Move move = ordering.selectMove(i);
		m_Board.makeMove(move, state);
		int moveScore = -qsearch(-beta, -alpha, depth - 1);
		m_Board.unmakeMove(move, state);

		if (moveScore >= beta)
			return beta;

		if (moveScore > alpha)
			alpha = moveScore;
	}

	return alpha;
}
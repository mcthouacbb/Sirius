#include "search.h"
#include "eval/eval.h"
#include "movegen.h"
#include "move_ordering.h"
#include "comm/move.h"
#include "comm/icomm.h"
#include "search_params.h"

#include <cstring>
#include <climits>
#include <algorithm>

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

SearchThread::SearchThread(uint32_t id, std::thread&& thread)
	: id(id), thread(std::move(thread))
{

}

void SearchThread::reset()
{
	memset(history, 0, sizeof(history));
	nodes = 0;
	rootPly = 0;
	checkCounter = TIME_CHECK_INTERVAL;

	for (int i = 0; i <= MAX_PLY; i++)
	{
		plies[i].killers[0] = plies[i].killers[1] = Move();
		plies[i].pv = nullptr;
		plies[i].pvLength = 0;
	}
}

Search::Search(Board& board)
	: m_Board(board), m_TT(2 * 1024 * 1024), m_WakeFlag(WakeFlag::NONE), m_ShouldStop(false), m_RunningThreads(0)
{
	setThreads(1);
}

Search::~Search()
{
	if (searching())
		stop();
	joinThreads();
}

void Search::storeKiller(SearchPly* ply, Move killer)
{
	if (ply->killers[0] != killer)
	{
		ply->killers[1] = ply->killers[0];
		ply->killers[0] = killer;
	}
}

void Search::newGame()
{
	for (auto& thread : m_Threads)
	{
		thread.reset();
	}
	m_TT.reset();
}

void Search::run(const SearchLimits& limits)
{
	m_WakeMutex.lock();

	m_TT.incAge();
	m_TimeMan.setLimits(limits, m_Board.sideToMove());
	m_TimeMan.startSearch();

	m_ShouldStop.store(false, std::memory_order_relaxed);

	m_WakeFlag.store(WakeFlag::SEARCH, std::memory_order_seq_cst);

	for (auto& thread : m_Threads)
	{
		thread.board.setState(m_Board);
		thread.limits = limits;
	}

	m_RunningThreads.store(static_cast<int>(m_Threads.size()), std::memory_order::seq_cst);

	m_WakeMutex.unlock();
	m_WakeCV.notify_all();
}

void Search::stop()
{
	m_ShouldStop.store(true, std::memory_order_relaxed);

	if (m_RunningThreads.load(std::memory_order_seq_cst) > 0)
	{
		std::unique_lock lock(m_StopMutex);
		m_StopCV.wait(lock, [this]
		{
			return m_RunningThreads.load(std::memory_order_seq_cst) == 0;
		});
	}
}

void Search::setThreads(int count)
{
	if (m_Threads.size() != count)
	{
		joinThreads();
		m_Threads.clear();
		m_Threads.reserve(count);
		for (int i = 0; i < count; i++)
		{
			m_Threads.emplace_back(i, std::thread());
			auto& searchThread = m_Threads.back();
			searchThread.thread = std::thread([this, &searchThread]
			{
				threadLoop(searchThread);
			});
		}
	}
}

bool Search::searching() const
{
	return m_RunningThreads.load(std::memory_order_seq_cst) != 0;
}

void Search::joinThreads()
{
	m_WakeFlag.store(WakeFlag::QUIT, std::memory_order_seq_cst);

	m_WakeCV.notify_all();

	for (auto& thread : m_Threads)
	{
		thread.thread.join();
	}

	m_WakeFlag.store(WakeFlag::NONE, std::memory_order_seq_cst);
}

void Search::threadLoop(SearchThread& thread)
{
	while (true)
	{
		std::unique_lock<std::mutex> uniqueLock(m_WakeMutex);
		WakeFlag flag;
		m_WakeCV.wait(uniqueLock, [&thread, &flag, this]
		{
			flag = m_WakeFlag.load(std::memory_order_seq_cst);
			return flag != WakeFlag::NONE;
		});


		switch (flag)
		{
			case WakeFlag::QUIT:
				return;
			case WakeFlag::SEARCH:
				iterDeep(thread, thread.isMainThread(), true);
				break;
			case WakeFlag::NONE:
				// unreachable;
				break;
		}
	}
}

int Search::iterDeep(SearchThread& thread, bool report, bool normalSearch)
{
	int maxDepth = std::min(thread.limits.maxDepth, MAX_PLY - 1);
	Move pv[MAX_PLY + 1];
	int score = 0;

	thread.reset();
	thread.checkCounter = TIME_CHECK_INTERVAL;

	report = report && normalSearch;

	for (int depth = 1; depth <= maxDepth; depth++)
	{
		thread.plies[0].pv = pv;
		int searchScore = aspWindows(thread, depth, score);
		if (m_ShouldStop)
			break;
		memcpy(thread.pv, thread.plies[0].pv, thread.plies[0].pvLength * sizeof(Move));
		score = searchScore;
		if (report)
		{
			SearchInfo info;
			info.nodes = thread.nodes;
			info.depth = depth;
			info.time = m_TimeMan.elapsed();
			info.pvBegin = pv;
			info.pvEnd = pv + thread.plies[0].pvLength;
			info.score = searchScore;
			if (report)
			{
				comm::currComm->reportSearchInfo(info);
			}
		}
	}

	if (report)
	{
		comm::currComm->reportBestMove(pv[0]);
	}

	if (normalSearch)
	{
		m_RunningThreads--;
		m_StopCV.notify_one();
		m_WakeFlag.store(WakeFlag::NONE, std::memory_order_seq_cst);
	}

	return score;
}

int Search::aspWindows(SearchThread& thread, int depth, int prevScore)
{
	int delta = ASP_INIT_DELTA;
	int alpha = prevScore - delta;
	int beta = prevScore + delta;

	while (true)
	{
		int searchScore = search(thread, depth, &thread.plies[0], alpha, beta, true);
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

	std::unique_ptr<SearchThread> thread = std::make_unique<SearchThread>(0, std::thread());
	thread->limits = limits;
	thread->board.setState(m_Board);

	iterDeep(*thread, false, false);

	BenchData data;
	data.nodes = thread->nodes;

	return data;
}

int Search::search(SearchThread& thread, int depth, SearchPly* searchPly, int alpha, int beta, bool isPV)
{
	if (--thread.checkCounter == 0)
	{
		thread.checkCounter = TIME_CHECK_INTERVAL;
		if (thread.isMainThread() && m_TimeMan.shouldStop(thread.limits))
		{
			m_ShouldStop.store(true, std::memory_order_relaxed);
			return alpha;
		}
	}

	if (m_ShouldStop.load(std::memory_order_relaxed))
		return alpha;

	thread.nodes++;

	auto& rootPly = thread.rootPly;
	auto& board = thread.board;
	auto& history = thread.history;


	alpha = std::max(alpha, -SCORE_MATE + rootPly);
	beta = std::min(beta, SCORE_MATE - rootPly);
	if (alpha >= beta)
		return alpha;

	bool root = rootPly == 0;

	if (eval::isImmediateDraw(board) || board.isDraw(rootPly))
	{
		searchPly->pvLength = 0;
		return SCORE_DRAW;
	}

	if (rootPly >= MAX_PLY)
	{
		searchPly->pvLength = 0;
		return eval::evaluate(board);
	}

	if (depth <= 0)
		return qsearch(thread, searchPly, alpha, beta);

	int hashScore = INT_MIN;
	Move hashMove = Move();
	TTBucket* bucket = m_TT.probe(board.zkey(), depth, rootPly, alpha, beta, hashScore, hashMove);

	if (hashScore != INT_MIN && !isPV)
	{
		searchPly->pvLength = 0;
		return hashScore;
	}

	int staticEval = eval::evaluate(board);
	BoardState state;

	if (!isPV && !board.checkers())
	{
		// reverse futility pruning
		if (depth <= RFP_MAX_DEPTH && staticEval >= beta + RFP_MARGIN * depth)
		{
			searchPly->pvLength = 0;
			return staticEval;
		}

		// null move pruning

		if (board.pliesFromNull() > 0)
		{
			BitBoard nonPawns = board.getColor(board.sideToMove()) ^ board.getPieces(board.sideToMove(), PieceType::PAWN);
			if ((nonPawns & (nonPawns - 1)) && depth >= NMP_MIN_DEPTH)
			{
				int r = NMP_BASE_REDUCTION;
				board.makeNullMove(state);
				rootPly++;
				int nullScore = -search(thread, depth - r, searchPly + 1, -beta, -beta + 1, false);
				rootPly--;
				board.unmakeNullMove();
				if (nullScore >= beta)
				{
					searchPly->pvLength = 0;
					return nullScore;
				}
			}
		}
	}

	Move moves[256];
	Move* end = genMoves<MoveGenType::LEGAL>(board, moves);

	if (moves == end)
	{
		searchPly->pvLength = 0;
		if (board.checkers())
			return -SCORE_MATE + rootPly;
		return SCORE_DRAW;
	}
	MoveOrdering ordering(
		board,
		moves,
		end,
		hashMove,
		searchPly->killers,
		thread.history[static_cast<int>(board.sideToMove())]
	);

	Move childPV[MAX_PLY + 1];
	searchPly[1].pv = childPV;

	searchPly->bestMove = Move();

	TTEntry::Bound bound = TTEntry::Bound::UPPER_BOUND;
	bool inCheck = board.checkers() != 0;

	Move quietsTried[256];
	int numQuietsTried = 0;

	int bestScore = -SCORE_MAX;

	for (uint32_t i = 0; i < end - moves; i++)
	{
		auto [move, moveScore] = ordering.selectMove(i);
		bool givesCheck = board.givesCheck(move);
		bool isCapture = board.getPieceAt(move.dstPos()) != PIECE_NONE;
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
				!inCheck &&
				depth <= LMP_MAX_DEPTH &&
				static_cast<int>(i) >= LMP_MIN_MOVES_BASE + depth * depth)
				break;

			if (!isPV &&
				depth <= MAX_SEE_PRUNE_DEPTH &&
				!board.see_margin(move, depth * SEE_PRUNE_MARGIN))
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
		board.makeMove(move, state);
		if (!isPromotion && !isCapture)
			quietsTried[numQuietsTried++] = move;
		rootPly++;

		int newDepth = depth + givesCheck - 1;
		int score;
		if (i == 0)
			score = -search(thread, newDepth, searchPly + 1, -beta, -alpha, isPV);
		else
		{
			score = -search(thread, newDepth - reduction, searchPly + 1, -(alpha + 1), -alpha, false);

			/*if (moveScore > alpha && reduction)
				moveScore = -search(newDepth, searchPly + 1, -(alpha + 1), -alpha, false);*/

			if (score > alpha && (isPV || reduction > 0))
				score = -search(thread, newDepth, searchPly + 1, -beta, -alpha, true);
		}
		rootPly--;
		board.unmakeMove(move);

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
					updateHistory(history[static_cast<int>(board.sideToMove())][historyIndex(move)], historyBonus);
					for (int j = 0; j < numQuietsTried - 1; j++)
					{
						updateHistory(history[static_cast<int>(board.sideToMove())][historyIndex(quietsTried[j])], -historyBonus);
					}
				}
				m_TT.store(bucket, board.zkey(), depth, rootPly, bestScore, move, TTEntry::Bound::LOWER_BOUND);
				return bestScore;
			}

			if (bestScore > alpha)
			{
				bound = TTEntry::Bound::EXACT;
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

	m_TT.store(bucket, board.zkey(), depth, rootPly, bestScore, searchPly->bestMove, bound);

	return bestScore;
}

int Search::qsearch(SearchThread& thread, SearchPly* searchPly, int alpha, int beta)
{
	auto& rootPly = thread.rootPly;
	auto& board = thread.board;
	searchPly->pvLength = 0;
	if (eval::isImmediateDraw(board))
		return SCORE_DRAW;

	int hashScore = INT_MIN;
	Move hashMove = Move();
	// qsearch is always depth 0
	TTBucket* bucket = m_TT.probe(board.zkey(), 0, rootPly, alpha, beta, hashScore, hashMove);

	if (hashScore != INT_MIN)
	{
		searchPly->pvLength = 0;
		return hashScore;
	}

	int eval = eval::evaluate(board);

	thread.nodes++;

	if (eval >= beta)
		return eval;
	if (eval > alpha)
		alpha = eval;

	if (rootPly >= MAX_PLY)
		return alpha;

	Move childPV[MAX_PLY + 1];
	searchPly[1].pv = childPV;

	Move captures[256];
	Move* end = genMoves<MoveGenType::CAPTURES>(board, captures);

	MoveOrdering ordering(board, captures, end, hashMove);

	BoardState state;
	TTEntry::Bound bound = TTEntry::Bound::UPPER_BOUND;
	searchPly->bestMove = Move();
	for (uint32_t i = 0; i < end - captures; i++)
	{
		auto [move, moveScore] = ordering.selectMove(i);
		if (!board.see_margin(move, 0))
			continue;
		board.makeMove(move, state);
		rootPly++;
		int score = -qsearch(thread, searchPly + 1, -beta, -alpha);
		board.unmakeMove(move);
		rootPly--;

		if (score > eval)
		{
			eval = score;

			if (eval >= beta)
			{
				m_TT.store(bucket, board.zkey(), 0, rootPly, eval, move, TTEntry::Bound::LOWER_BOUND);
				return eval;
			}

			if (eval > alpha)
			{
				searchPly->bestMove = move;

				searchPly->pvLength = searchPly[1].pvLength + 1;
				searchPly->pv[0] = move;
				memcpy(searchPly->pv + 1, searchPly[1].pv, searchPly[1].pvLength * sizeof(Move));

				alpha = eval;
				bound = TTEntry::Bound::EXACT;
			}
		}
	}

	m_TT.store(bucket, board.zkey(), 0, rootPly, eval, searchPly->bestMove, bound);


	return eval;
}


}

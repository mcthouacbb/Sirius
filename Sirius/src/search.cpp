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
#include <cmath>

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

// initialize wakeFlag to search to allow for waiting on search thread at init
SearchThread::SearchThread(uint32_t id, std::thread&& thread)
	: id(id), thread(std::move(thread)), wakeFlag(WakeFlag::SEARCH), limits(), pv(), history(), plies()
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

void SearchThread::startSearching()
{
	mutex.lock();
	wakeFlag = WakeFlag::SEARCH;
	mutex.unlock();
	cv.notify_one();
}

void SearchThread::wait()
{
	std::unique_lock<std::mutex> uniqueLock(mutex);
	cv.wait(uniqueLock, [&]
	{
		return wakeFlag == WakeFlag::NONE;
	});
}

void SearchThread::join()
{
	mutex.lock();
	wakeFlag = WakeFlag::QUIT;
	mutex.unlock();
	cv.notify_one();
	thread.join();
}

Search::Search(Board& board)
<<<<<<< Updated upstream
	: m_Board(board), m_ShouldStop(false), m_TT(2 * 1024 * 1024)
||||||| constructed merge base
	: m_Board(board), m_TT(1024 * 1024), m_RootPly(0)
=======
	: m_Board(board), m_TT(8 * 1024 * 1024), m_RootPly(0)
>>>>>>> Stashed changes
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
		thread->reset();
	}
	m_TT.reset();
}

void Search::run(const SearchLimits& limits, const std::deque<BoardState>& states)
{
	// wait for all threads to finish before starting search
	for (auto& thread : m_Threads)
	{
		thread->wait();
	}

	m_TT.incAge();

	m_ShouldStop.store(false, std::memory_order_relaxed);

	m_TimeMan.setLimits(limits, m_Board.sideToMove());
	m_TimeMan.startSearch();

	m_States = states;
	// loop through all states except for root
	for (auto it = m_States.rbegin(); it != m_States.rend() - 1; it++)
		// horrible abomination
		it->prev = &(*(it + 1));


	for (auto& thread : m_Threads)
	{
		thread->board.setState(m_Board, m_States.back(), m_States.front());
		thread->limits = limits;

		// wait for thread to finish loop if it hasn't yet
		thread->startSearching();
	}
}

void Search::stop()
{
	m_ShouldStop.store(true, std::memory_order_relaxed);

	for (auto& thread : m_Threads)
	{
		thread->wait();
	}
}

void Search::setThreads(int count)
{
	if (static_cast<int>(m_Threads.size()) != count)
	{
		joinThreads();
		m_Threads.clear();
		m_Threads.reserve(count);
		for (int i = 0; i < count; i++)
		{
			m_Threads.push_back(std::make_unique<SearchThread>(i, std::thread()));
			auto& searchThread = m_Threads.back();
			searchThread->thread = std::thread([this, &searchThread]
			{
				threadLoop(*searchThread);
			});
			searchThread->wait();
		}
	}
}

bool Search::searching() const
{
	for (auto& thread : m_Threads)
	{
		std::lock_guard<std::mutex> guard(thread->mutex);
		if (thread->wakeFlag == WakeFlag::SEARCH)
			return true;
	}
	return false;
}

void Search::joinThreads()
{
	for (auto& thread : m_Threads)
	{
		thread->join();
	}
}

void Search::threadLoop(SearchThread& thread)
{
	while (true)
	{
		std::unique_lock<std::mutex> uniqueLock(thread.mutex);
		// notify after acquiring wakeMutex
		thread.cv.notify_one();

		thread.wakeFlag = WakeFlag::NONE;
		thread.cv.wait(uniqueLock, [&thread, this]
		{
			return thread.wakeFlag != WakeFlag::NONE;
		});

		WakeFlag flag = thread.wakeFlag;
		uniqueLock.unlock();

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
	Move pv[MAX_PLY + 1] = {};
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

BenchData Search::benchSearch(int depth, const Board& board, BoardState& state)
{
	SearchLimits limits = {};
	limits.maxDepth = depth;

	std::unique_ptr<SearchThread> thread = std::make_unique<SearchThread>(0, std::thread());
	thread->limits = limits;
	thread->board.setState(board, state, state);

	iterDeep(*thread, false, false);

	BenchData data = {};
	data.nodes = thread->nodes;

	return data;
}

int Search::search(SearchThread& thread, int depth, SearchPly* searchPly, int alpha, int beta, bool isPV)
{
	if (--thread.checkCounter == 0)
	{
		thread.checkCounter = TIME_CHECK_INTERVAL;
		if (m_TimeMan.shouldStop(thread.limits))
		{
			m_ShouldStop = true;
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

<<<<<<< Updated upstream
	int hashScore = INT_MIN;
	Move hashMove = Move();
	TTBucket* bucket = m_TT.probe(board.zkey(), depth, rootPly, alpha, beta, hashScore, hashMove);
||||||| constructed merge base
	int hashScore = INT_MIN;
	Move hashMove = Move();
	TTBucket* bucket = m_TT.probe(m_Board.zkey(), depth, m_RootPly, alpha, beta, hashScore, hashMove);
=======
	int ttScore = INT_MIN;
	Move ttMove = Move();
	TTEntry* entry = m_TT.probe(m_Board.zkey(), m_RootPly, ttScore, ttMove);
>>>>>>> Stashed changes

	if (ttScore != INT_MIN && !isPV && depth >= entry->depth &&
		(entry->bound() == TTEntry::Bound::EXACT ||
		entry->bound() == TTEntry::Bound::LOWER && alpha >= ttScore ||
		entry->bound() == TTEntry::Bound::UPPER && beta <= ttScore))
	{
		searchPly->pvLength = 0;
		return ttScore;
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
		ttMove,
		searchPly->killers,
		thread.history[static_cast<int>(board.sideToMove())]
	);

	Move childPV[MAX_PLY + 1];
	searchPly[1].pv = childPV;

	searchPly->bestMove = Move();

<<<<<<< Updated upstream
	TTEntry::Bound bound = TTEntry::Bound::UPPER_BOUND;
	bool inCheck = board.checkers() != 0;
||||||| constructed merge base
	TTEntry::Type type = TTEntry::Type::UPPER_BOUND;
	bool inCheck = m_Board.checkers() != 0;
=======
	TTEntry::Bound bound = TTEntry::Bound::UPPER;
	bool inCheck = m_Board.checkers() != 0;
>>>>>>> Stashed changes

	Move quietsTried[256];
	int numQuietsTried = 0;

	int bestScore = -SCORE_MAX;

	for (uint32_t i = 0; i < end - moves; i++)
	{
		auto [move, moveScore] = ordering.selectMove(i);
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
		board.makeMove(move, state);
		bool givesCheck = board.checkers() != 0;
		if (!isPromotion && !isCapture)
			quietsTried[numQuietsTried++] = move;
		rootPly++;

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
<<<<<<< Updated upstream
				m_TT.store(bucket, board.zkey(), depth, rootPly, bestScore, move, TTEntry::Bound::LOWER_BOUND);
||||||| constructed merge base
				m_TT.store(bucket, m_Board.zkey(), depth, m_RootPly, bestScore, move, TTEntry::Type::LOWER_BOUND);
=======
				m_TT.store(entry, m_Board.zkey(), depth, m_RootPly, bestScore, move, TTEntry::Bound::LOWER);
>>>>>>> Stashed changes
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

<<<<<<< Updated upstream
	m_TT.store(bucket, board.zkey(), depth, rootPly, bestScore, searchPly->bestMove, bound);
||||||| constructed merge base
	m_TT.store(bucket, m_Board.zkey(), depth, m_RootPly, bestScore, searchPly->bestMove, type);
=======
	m_TT.store(entry, m_Board.zkey(), depth, m_RootPly, bestScore, searchPly->bestMove, bound);
>>>>>>> Stashed changes

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

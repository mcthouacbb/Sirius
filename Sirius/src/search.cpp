#include "search.h"
#include "eval/eval.h"
#include "move_ordering.h"
#include "movegen.h"
#include "search_params.h"
#include "uci/uci.h"

#include <algorithm>
#include <climits>
#include <cmath>
#include <cstring>

namespace search
{

MultiArray<i32, 64, 64> genLMRTable()
{
    std::array<std::array<i32, 64>, 64> lmrTable = {};
    for (i32 d = 1; d < 64; d++)
    {
        for (i32 i = 1; i < 64; i++)
        {
            f64 base = static_cast<f64>(lmrBase);
            f64 scale = static_cast<f64>(lmrScale);
            lmrTable[d][i] = static_cast<i32>(
                base + scale * std::log(static_cast<f64>(d)) * std::log(static_cast<f64>(i)));
        }
    }
    return lmrTable;
}

MultiArray<i32, 64, 64> lmrTable = {};

void init()
{
    lmrTable = genLMRTable();
}

// initialize wakeFlag to search to allow for waiting on search thread at init
SearchThread::SearchThread(u32 id, std::thread&& thread)
    : id(id), thread(std::move(thread)), wakeFlag(WakeFlag::SEARCH), limits(), stack()
{
}

void SearchThread::reset()
{
    nodes = 0;
    rootPly = 0;

    for (i32 i = 0; i <= MAX_PLY; i++)
    {
        stack[i].pv = {};
        stack[i].pvLength = 0;
    }
}

void SearchThread::startSearching()
{
    mutex.lock();
    wakeFlag = WakeFlag::SEARCH;
    mutex.unlock();
    cv.notify_one();
}

void SearchThread::initRootMoves()
{
    rootMoves.clear();
    MoveList moves;
    genMoves<MoveGenType::LEGAL>(board, moves);
    for (Move move : moves)
    {
        rootMoves.push_back(RootMove(move));
    }
}

void SearchThread::sortRootMoves()
{
    std::stable_sort(rootMoves.begin(), rootMoves.end(),
        [](const RootMove& a, const RootMove& b)
        {
            return a.score == b.score ? a.previousScore > b.previousScore : a.score > b.score;
        });
}

RootMove& SearchThread::findRootMove(Move move)
{
    auto it = std::find_if(rootMoves.begin(), rootMoves.end(),
        [=](const RootMove& rm)
        {
            return rm.move == move;
        });
    return *it;
}

void SearchThread::wait()
{
    std::unique_lock<std::mutex> uniqueLock(mutex);
    cv.wait(uniqueLock,
        [&]
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

Search::Search()
    : m_ShouldStop(false)
{
    setThreads(1);
}

Search::~Search()
{
    if (searching())
        stop();
    joinThreads();
}

void Search::newGame()
{
    for (auto& thread : m_Threads)
    {
        thread->reset();
        thread->pawnTable.clear();
    }
}

void Search::run(const SearchLimits& limits, const Board& board)
{
    // wait for all threads to finish before starting search
    for (auto& thread : m_Threads)
    {
        thread->wait();
    }

    m_ShouldStop.store(false, std::memory_order_relaxed);

    m_TimeMan.setLimits(limits, board.sideToMove());
    m_TimeMan.startSearch();

    for (auto& thread : m_Threads)
    {
        thread->board = board;
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

void Search::setThreads(i32 count)
{
    if (static_cast<i32>(m_Threads.size()) != count)
    {
        joinThreads();
        m_Threads.clear();
        m_Threads.reserve(count);
        for (i32 i = 0; i < count; i++)
        {
            m_Threads.push_back(std::make_unique<SearchThread>(i, std::thread()));
            auto& searchThread = m_Threads.back();
            searchThread->thread = std::thread(
                [this, &searchThread]
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
        thread.cv.wait(uniqueLock,
            [&thread]
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
                iterDeep(thread, thread.isMainThread());
                break;
            case WakeFlag::NONE:
                // unreachable;
                break;
        }
    }
}

void Search::makeMove(SearchThread& thread, SearchStack* stack, Move move)
{
    thread.rootPly++;
    thread.board.makeMove(move, thread.evalState);
    thread.nodes.fetch_add(1, std::memory_order_relaxed);
}

void Search::unmakeMove(SearchThread& thread, SearchStack* stack)
{
    thread.rootPly--;
    thread.board.unmakeMove(thread.evalState);
}

void Search::makeNullMove(SearchThread& thread, SearchStack* stack)
{
    thread.board.makeNullMove();
    thread.rootPly++;
}

void Search::unmakeNullMove(SearchThread& thread, SearchStack* stack)
{
    thread.rootPly--;
    thread.board.unmakeNullMove();
}

void Search::reportUCIInfo(const SearchThread& thread, i32 multiPVIdx, i32 depth) const
{
    SearchInfo info;
    info.nodes = 0;
    for (auto& searchThread : m_Threads)
        info.nodes += searchThread->nodes.load(std::memory_order_relaxed);
    info.depth = depth;
    info.selDepth = thread.rootMoves[multiPVIdx].selDepth;
    info.hashfull = 0;
    info.time = m_TimeMan.elapsed();
    info.pv = thread.rootMoves[multiPVIdx].pv;
    info.score = thread.rootMoves[multiPVIdx].displayScore;
    info.lowerbound = thread.rootMoves[multiPVIdx].lowerbound;
    info.upperbound = thread.rootMoves[multiPVIdx].upperbound;
    uci::uci->reportSearchInfo(info);
}

std::pair<i32, Move> Search::iterDeep(SearchThread& thread, bool report)
{
    i32 maxDepth = std::min(thread.limits.maxDepth, MAX_PLY - 1);
    i32 score = 0;

    thread.reset();
    thread.evalState.init(thread.board, thread.pawnTable);
    thread.initRootMoves();

    for (i32 depth = 1; depth <= maxDepth; depth++)
    {
        thread.rootDepth = depth;
        thread.selDepth = 0;
        i32 searchScore = search(thread, depth, &thread.stack[0], -SCORE_MAX, SCORE_MAX);
        thread.sortRootMoves();
        if (m_ShouldStop)
            break;
        score = searchScore;
        if (report)
            reportUCIInfo(thread, 0, depth);

        if (thread.isMainThread()
            && m_TimeMan.stopSoft(thread.rootMoves[0].move, thread.nodes, thread.limits))
            break;
    }

    if (thread.isMainThread())
        m_ShouldStop.store(true, std::memory_order_relaxed);

    if (report)
        uci::uci->reportBestMove(thread.rootMoves[0].move);

    return {score, thread.rootMoves[0].move};
}

BenchData Search::benchSearch(i32 depth, const Board& board)
{
    SearchLimits limits = {};
    limits.maxDepth = depth;

    std::unique_ptr<SearchThread> thread = std::make_unique<SearchThread>(0, std::thread());
    thread->limits = limits;
    thread->board = board;

    m_TimeMan.setLimits(limits, board.sideToMove());
    m_TimeMan.startSearch();

    m_ShouldStop.store(false, std::memory_order_relaxed);

    iterDeep(*thread, false);

    BenchData data = {};
    data.nodes = thread->nodes;

    return data;
}

std::pair<i32, Move> Search::datagenSearch(const SearchLimits& limits, const Board& board)
{
    std::unique_ptr<SearchThread> thread = std::make_unique<SearchThread>(0, std::thread());
    thread->limits = limits;
    thread->board = board;
    m_TimeMan.setLimits(limits, board.sideToMove());
    m_TimeMan.startSearch();

    m_ShouldStop.store(false, std::memory_order_relaxed);

    return iterDeep(*thread, false);
}

i32 Search::search(SearchThread& thread, i32 depth, SearchStack* stack, i32 alpha, i32 beta)
{
    if (thread.isMainThread() && m_TimeMan.stopHard(thread.limits, thread.nodes))
    {
        m_ShouldStop = true;
        return alpha;
    }

    if (m_ShouldStop.load(std::memory_order_relaxed))
        return alpha;

    auto& rootPly = thread.rootPly;
    auto& board = thread.board;

    if (rootPly + 1 > thread.selDepth)
        thread.selDepth = rootPly + 1;

    alpha = std::max(alpha, -SCORE_MATE + rootPly);
    beta = std::min(beta, SCORE_MATE - rootPly);
    if (alpha >= beta)
        return alpha;

    stack->pvLength = 0;

    bool root = rootPly == 0;
    bool inCheck = board.checkers().any();

    if (board.isDraw(rootPly))
        return SCORE_DRAW;

    if (rootPly >= MAX_PLY)
        return eval::evaluate(board, &thread);

    if (depth <= 0)
        return eval::evaluate(board, &thread);

    MoveOrdering ordering(board);

    Move bestMove = Move::nullmove();

    i32 bestScore = -SCORE_MAX;
    i32 movesPlayed = 0;

    ScoredMove scoredMove = {};
    while ((scoredMove = ordering.selectMove()).score != MoveOrdering::NO_MOVE)
    {
        auto [move, moveScore] = scoredMove;
        if (!board.isLegal(move))
            continue;

        u64 nodesBefore = thread.nodes;

        makeMove(thread, stack, move);
        movesPlayed++;

        i32 newDepth = depth - 1;
        i32 score = -search(thread, newDepth, stack + 1, -beta, -alpha);

        unmakeMove(thread, stack);

        if (m_ShouldStop)
            return alpha;

        if (root)
        {
            RootMove& rootMove = thread.findRootMove(move);
            rootMove.previousScore = rootMove.score;
            rootMove.nodes += thread.nodes - nodesBefore;

            if (movesPlayed == 1 || score > alpha)
            {
                rootMove.displayScore = rootMove.score = score;
                rootMove.selDepth = thread.selDepth;
                rootMove.lowerbound = false;
                rootMove.upperbound = false;

                if (score >= beta)
                {
                    rootMove.displayScore = beta;
                    rootMove.lowerbound = true;
                }
                else if (score <= alpha)
                {
                    rootMove.displayScore = alpha;
                    rootMove.upperbound = true;
                }

                rootMove.pv = {move};
                for (i32 i = 0; i < (stack + 1)->pvLength; i++)
                    rootMove.pv.push_back((stack + 1)->pv[i]);
            }
            else
            {
                rootMove.score = SCORE_NONE;
            }
        }

        if (score > bestScore)
        {
            bestScore = score;

            if (bestScore > alpha)
            {
                alpha = bestScore;
                bestMove = move;

                stack->pv[0] = move;
                stack->pvLength = (stack + 1)->pvLength + 1;
                for (i32 i = 0; i < (stack + 1)->pvLength; i++)
                    stack->pv[i + 1] = (stack + 1)->pv[i];
            }

            if (bestScore >= beta)
                break;
        }
    }

    if (movesPlayed == 0)
    {
        if (inCheck)
            return -SCORE_MATE + rootPly;
        return SCORE_DRAW;
    }

    return bestScore;
}

}

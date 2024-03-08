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

std::array<std::array<int, 64>, 64> lmrTable;

void init()
{
    for (int d = 1; d < 64; d++)
    {
        for (int i = 1; i < 64; i++)
        {
            lmrTable[d][i] = static_cast<int>(lmrBase / 100.0 + std::log(static_cast<double>(d)) * std::log(static_cast<double>(i)) / (lmrDivisor / 100.0));
        }
    }
}

// initialize wakeFlag to search to allow for waiting on search thread at init
SearchThread::SearchThread(uint32_t id, std::thread&& thread)
    : id(id), thread(std::move(thread)), wakeFlag(WakeFlag::SEARCH), limits(), stack(), history()
{

}

void SearchThread::reset()
{
    nodes = 0;
    rootPly = 0;

    for (int i = 0; i <= MAX_PLY; i++)
    {
        stack[i].killers[0] = stack[i].killers[1] = Move();
        stack[i].pv = {};
        stack[i].pvLength = 0;
        stack[i].contHistEntry = nullptr;
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
    : m_Board(board), m_ShouldStop(false), m_TT(2 * 1024 * 1024)
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
        thread->history.clear();
        thread->pawnTable.clear();
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
        thread.cv.wait(uniqueLock, [&thread]
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
    int score = 0;
    Move bestMove = {};

    thread.reset();

    report = report && normalSearch;

    for (int depth = 1; depth <= maxDepth; depth++)
    {
        thread.selDepth = 0;
        int searchScore = aspWindows(thread, depth, bestMove, score);
        if (m_ShouldStop)
            break;
        score = searchScore;
        if (report)
        {
            SearchInfo info;
            info.nodes = 0;
            for (auto& searchThread : m_Threads)
                info.nodes += searchThread->nodes.load(std::memory_order_relaxed);
            info.depth = depth;
            info.selDepth = thread.selDepth;
            info.time = m_TimeMan.elapsed();
            info.pvBegin = thread.stack[0].pv.data();
            info.pvEnd = thread.stack[0].pv.data() + thread.stack[0].pvLength;
            info.score = searchScore;
            comm::currComm->reportSearchInfo(info);
        }
        if (thread.isMainThread() && m_TimeMan.stopSoft(bestMove, thread.nodes, thread.limits))
        {
            m_ShouldStop.store(true, std::memory_order_relaxed);
            break;
        }
    }

    if (report)
    {
        comm::currComm->reportBestMove(bestMove);
    }

    return score;
}

int Search::aspWindows(SearchThread& thread, int depth, Move& bestMove, int prevScore)
{
    int delta = aspInitDelta;
    int alpha = -SCORE_MAX;
    int beta = SCORE_MAX;

    if (depth >= minAspDepth)
    {
        alpha = prevScore - delta;
        beta = prevScore + delta;
    }

    while (true)
    {
        int searchScore = search(thread, depth, &thread.stack[0], alpha, beta, true);
        if (m_ShouldStop)
            return searchScore;

        if (searchScore <= alpha)
        {
            beta = (alpha + beta) / 2;
            alpha -= delta;
        }
        else
        {
            bestMove = thread.stack[0].pv[0];
            if (searchScore >= beta)
                beta += delta;
            else
                return searchScore;
        }
        delta += delta * aspWideningFactor / 16;
    }
}

BenchData Search::benchSearch(int depth, const Board& board, BoardState& state)
{
    SearchLimits limits = {};
    limits.maxDepth = depth;

    std::unique_ptr<SearchThread> thread = std::make_unique<SearchThread>(0, std::thread());
    thread->limits = limits;
    thread->board.setState(board, state, state);

    m_TimeMan.setLimits(limits, m_Board.sideToMove());
    m_TimeMan.startSearch();

    m_ShouldStop.store(false, std::memory_order_relaxed);

    iterDeep(*thread, false, false);

    BenchData data = {};
    data.nodes = thread->nodes;

    return data;
}

int Search::search(SearchThread& thread, int depth, SearchStack* stack, int alpha, int beta, bool isPV)
{
    if (m_TimeMan.stopHard(thread.limits, thread.nodes))
    {
        m_ShouldStop = true;
        return alpha;
    }

    if (m_ShouldStop.load(std::memory_order_relaxed))
        return alpha;

    auto& rootPly = thread.rootPly;
    auto& board = thread.board;
    auto& history = thread.history;

    if (rootPly > thread.selDepth)
        thread.selDepth = rootPly;

    alpha = std::max(alpha, -SCORE_MATE + rootPly);
    beta = std::min(beta, SCORE_MATE - rootPly);
    if (alpha >= beta)
        return alpha;

    stack->pvLength = 0;

    bool root = rootPly == 0;
    bool inCheck = board.checkers().any();

    if (eval::isImmediateDraw(board) || board.isDraw(rootPly))
        return SCORE_DRAW;

    if (rootPly >= MAX_PLY)
        return eval::evaluate(board, &thread);

    if (depth <= 0)
        return qsearch(thread, stack, alpha, beta);

    int ttScore;
    Move ttMove = Move();
    int ttDepth;
    TTEntry::Bound ttBound;
    bool ttHit;
    size_t ttIdx = m_TT.probe(board.zkey(), ttHit, rootPly, ttScore, ttMove, ttDepth, ttBound);

    if (ttHit)
    {
        if (!isPV && ttDepth >= depth && (
            ttBound == TTEntry::Bound::EXACT ||
            (ttBound == TTEntry::Bound::LOWER_BOUND && ttScore >= beta) ||
            (ttBound == TTEntry::Bound::UPPER_BOUND && ttScore <= alpha)
        ))
            return ttScore;
    }
    else if (depth >= minIIRDepth)
        depth--;

    stack->staticEval = inCheck ? SCORE_NONE : eval::evaluate(board);
    bool improving = !inCheck && rootPly > 1 && stack->staticEval > stack[-2].staticEval;

    int posEval = stack->staticEval;
    if (!inCheck && ttHit && (
        ttBound == TTEntry::Bound::EXACT ||
        (ttBound == TTEntry::Bound::LOWER_BOUND && ttScore >= posEval) ||
        (ttBound == TTEntry::Bound::UPPER_BOUND && ttScore <= posEval)
    ))
        posEval = ttScore;

    stack[1].killers = {};

    BoardState state;

    if (!isPV && !inCheck)
    {
        // reverse futility pruning
        if (depth <= rfpMaxDepth && posEval >= beta + (improving ? rfpImprovingMargin : rfpMargin) * depth)
            return posEval;

        // null move pruning
        Bitboard nonPawns = board.getColor(board.sideToMove()) ^ board.getPieces(board.sideToMove(), PieceType::PAWN);
        if (board.pliesFromNull() > 0 &&  depth >= nmpMinDepth && posEval >= beta && nonPawns.multiple())
        {
            int r = nmpBaseReduction + depth / nmpDepthReductionScale + std::min((posEval - beta) / nmpEvalReductionScale, nmpMaxEvalReduction);
            board.makeNullMove(state);
            rootPly++;
            int nullScore = -search(thread, depth - r, stack + 1, -beta, -beta + 1, false);
            rootPly--;
            board.unmakeNullMove();
            if (nullScore >= beta)
                return nullScore;
        }
    }

    MoveList moves;
    genMoves<MoveGenType::NOISY_QUIET>(board, moves);

    std::array<CHEntry*, 3> contHistEntries = {
        rootPly > 0 ? stack[-1].contHistEntry : nullptr,
        rootPly > 1 ? stack[-2].contHistEntry : nullptr,
        rootPly > 3 ? stack[-4].contHistEntry : nullptr
    };

    MoveOrdering ordering(
        board,
        moves,
        ttMove,
        stack->killers,
        contHistEntries,
        thread.history
    );

    stack[1].failHighCount = 0;

    stack->bestMove = Move();

    TTEntry::Bound bound = TTEntry::Bound::UPPER_BOUND;

    MoveList quietsTried;

    int bestScore = -SCORE_MAX;
    int movesPlayed = 0;

    for (int moveIdx = 0; moveIdx < static_cast<int>(moves.size()); moveIdx++)
    {
        auto [move, moveScore, moveHistory] = ordering.selectMove(static_cast<uint32_t>(moveIdx));
        if (!board.isLegal(move))
            continue;
        bool quiet = moveIsQuiet(board, move);
        bool quietLosing = moveScore < MoveOrdering::KILLER_SCORE;

        int baseLMR = lmrTable[std::min(depth, 63)][std::min(movesPlayed, 63)];
        if (quiet)
            baseLMR -= moveHistory / lmrHistDivisor;

        if (!root && quietLosing && bestScore > -SCORE_WIN)
        {
            int lmrDepth = std::max(depth - baseLMR, 0);
            if (lmrDepth <= fpMaxDepth &&
                !inCheck &&
                alpha < SCORE_WIN &&
                posEval + fpBaseMargin + fpDepthMargin * lmrDepth <= alpha)
            {
                continue;
            }

            if (!isPV &&
                !inCheck &&
                depth <= lmpMaxDepth &&
                movesPlayed >= lmpMinMovesBase + depth * depth / (improving ? 1 : 2))
                break;

            if (!isPV &&
                depth <= maxSeePruneDepth &&
                !board.see(move, depth * (quiet ? seePruneMarginQuiet : seePruneMarginNoisy)))
                continue;

            if (quiet &&
                depth <= maxHistPruningDepth &&
                moveHistory < -histPruningMargin * depth)
                break;
        }

        m_TT.prefetch(board.keyAfter(move));
        stack->contHistEntry = &history.contHistEntry(ExtMove::from(board, move));

        uint64_t nodesBefore = thread.nodes;
        board.makeMove(move, state);
        thread.nodes.fetch_add(1, std::memory_order_relaxed);
        bool givesCheck = board.checkers().any();
        if (quiet)
            quietsTried.push_back(move);
        rootPly++;

        int reduction = 0;
        if (movesPlayed >= (isPV ? lmrMinMovesPv : lmrMinMovesNonPv) &&
            depth >= lmrMinDepth &&
            quietLosing &&
            !inCheck)
        {
            reduction = baseLMR;

            reduction += !improving;
            reduction -= isPV;
            reduction -= givesCheck;

            reduction += stack[1].failHighCount >= static_cast<uint32_t>(lmrFailHighCountMargin);

            reduction = std::clamp(reduction, 0, depth - 2);
        }

        movesPlayed++;

        int newDepth = depth + givesCheck - 1;
        int score;
        if (movesPlayed == 1)
            score = -search(thread, newDepth, stack + 1, -beta, -alpha, isPV);
        else
        {
            score = -search(thread, newDepth - reduction, stack + 1, -(alpha + 1), -alpha, false);

            if (score > alpha && reduction > 0)
                score = -search(thread, newDepth, stack + 1, -(alpha + 1), -alpha, false);

            if (score > alpha && isPV)
                score = -search(thread, newDepth, stack + 1, -beta, -alpha, true);
        }
        rootPly--;
        board.unmakeMove(move);
        if (root && thread.isMainThread())
            m_TimeMan.updateNodes(move, thread.nodes - nodesBefore);

        stack->contHistEntry = nullptr;

        if (m_ShouldStop)
            return alpha;

        if (score > bestScore)
        {
            bestScore = score;

            if (bestScore > alpha)
            {
                bound = TTEntry::Bound::EXACT;
                alpha = bestScore;
                stack->bestMove = move;
                if (isPV)
                {
                    stack->pv[0] = move;
                    stack->pvLength = stack[1].pvLength + 1;
                    for (int i = 0; i < stack[1].pvLength; i++)
                        stack->pv[i + 1] = stack[1].pv[i];
                }
            }

            if (bestScore >= beta)
            {
                stack->failHighCount++;
                if (quiet)
                {
                    if (stack->killers[0] != move)
                    {
                        stack->killers[1] = stack->killers[0];
                        stack->killers[0] = move;
                    }

                    int bonus = historyBonus(depth);
                    history.updateQuietStats(ExtMove::from(board, move), contHistEntries, bonus);
                    for (int j = 0; j < static_cast<int>(quietsTried.size() - 1); j++)
                    {
                        history.updateQuietStats(ExtMove::from(board, quietsTried[j]), contHistEntries, -bonus);
                    }
                }
                m_TT.store(ttIdx, board.zkey(), depth, rootPly, bestScore, move, TTEntry::Bound::LOWER_BOUND);
                return bestScore;
            }
        }
    }

    if (movesPlayed == 0)
    {
        if (inCheck)
            return -SCORE_MATE + rootPly;
        return SCORE_DRAW;
    }

    m_TT.store(ttIdx, board.zkey(), depth, rootPly, bestScore, stack->bestMove, bound);

    return bestScore;
}

int Search::qsearch(SearchThread& thread, SearchStack* stack, int alpha, int beta)
{
    auto& rootPly = thread.rootPly;
    auto& board = thread.board;
    stack->pvLength = 0;
    if (eval::isImmediateDraw(board))
        return SCORE_DRAW;

    if (rootPly > thread.selDepth)
        thread.selDepth = rootPly;

    int ttScore;
    Move ttMove = Move();
    int ttDepth;
    TTEntry::Bound ttBound;
    bool ttHit;
    // qsearch is always depth 0
    size_t ttIdx = m_TT.probe(board.zkey(), ttHit, rootPly, ttScore, ttMove, ttDepth, ttBound);

    if (ttHit)
    {
        if (ttBound == TTEntry::Bound::EXACT ||
            (ttBound == TTEntry::Bound::LOWER_BOUND && ttScore >= beta) ||
            (ttBound == TTEntry::Bound::UPPER_BOUND && ttScore <= alpha)
        )
            return ttScore;
    }

    bool inCheck = board.checkers().any();
    int staticEval = inCheck ? SCORE_NONE : eval::evaluate(board, &thread);

    int posEval = staticEval;
    if (!inCheck && ttHit && (
        ttBound == TTEntry::Bound::EXACT ||
        (ttBound == TTEntry::Bound::LOWER_BOUND && ttScore >= posEval) ||
        (ttBound == TTEntry::Bound::UPPER_BOUND && ttScore <= posEval)
    ))
        posEval = ttScore;

    if (posEval >= beta)
        return posEval;
    if (posEval > alpha)
        alpha = posEval;

    int bestScore = inCheck ? -SCORE_MATE : posEval;

    if (rootPly >= MAX_PLY)
        return alpha;

    MoveList captures;
    genMoves<MoveGenType::NOISY>(board, captures);

    MoveOrdering ordering(board, captures, ttMove);

    BoardState state;
    TTEntry::Bound bound = TTEntry::Bound::UPPER_BOUND;
    stack->bestMove = Move();
    for (int moveIdx = 0; moveIdx < static_cast<int>(captures.size()); moveIdx++)
    {
        auto [move, moveScore, moveHistory] = ordering.selectMove(moveIdx);
        if (!board.isLegal(move))
            continue;
        if (!board.see(move, 0))
            continue;
        board.makeMove(move, state);
        thread.nodes.fetch_add(1, std::memory_order_relaxed);
        rootPly++;
        int score = -qsearch(thread, stack + 1, -beta, -alpha);
        board.unmakeMove(move);
        rootPly--;

        if (score > bestScore)
        {
            bestScore = score;

            if (bestScore >= beta)
            {
                m_TT.store(ttIdx, board.zkey(), 0, rootPly, bestScore, move, TTEntry::Bound::LOWER_BOUND);
                return bestScore;
            }

            if (bestScore > alpha)
            {
                stack->bestMove = move;

                stack->pvLength = stack[1].pvLength + 1;
                stack->pv[0] = move;
                for (int i = 0; i < stack[1].pvLength; i++)
                    stack->pv[i + 1] = stack[1].pv[i];

                alpha = bestScore;
                bound = TTEntry::Bound::EXACT;
            }
        }
    }

    m_TT.store(ttIdx, board.zkey(), 0, rootPly, bestScore, stack->bestMove, bound);


    return bestScore;
}


}

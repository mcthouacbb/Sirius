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
            lmrTable[d][i] = static_cast<int>(LMR_BASE + std::log(static_cast<double>(d)) * std::log(static_cast<double>(i)) / LMR_DIVISOR);
        }
    }
}

// initialize wakeFlag to search to allow for waiting on search thread at init
SearchThread::SearchThread(uint32_t id, std::thread&& thread)
    : id(id), thread(std::move(thread)), wakeFlag(WakeFlag::SEARCH), limits(), pv(), plies(), history()
{

}

void SearchThread::reset()
{
    nodes = 0;
    rootPly = 0;
    checkCounter = TIME_CHECK_INTERVAL;

    for (int i = 0; i <= MAX_PLY; i++)
    {
        plies[i].killers[0] = plies[i].killers[1] = Move();
        plies[i].pv = nullptr;
        plies[i].pvLength = 0;
        plies[i].contHistEntry = nullptr;
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
        thread->history.clear();
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
    thread.checkCounter = TIME_CHECK_INTERVAL;

    report = report && normalSearch;

    for (int depth = 1; depth <= maxDepth; depth++)
    {
        thread.plies[0].pv = thread.pv.data();
        int searchScore = aspWindows(thread, depth, bestMove, score);
        if (m_ShouldStop)
            break;
        score = searchScore;
        if (report)
        {
            SearchInfo info;
            info.nodes = thread.nodes;
            info.depth = depth;
            info.time = m_TimeMan.elapsed();
            info.pvBegin = thread.pv.data();
            info.pvEnd = thread.pv.data() + thread.plies[0].pvLength;
            info.score = searchScore;
            if (report)
            {
                comm::currComm->reportSearchInfo(info);
            }
        }
        if (m_TimeMan.stopSoft(bestMove, thread.nodes, thread.limits))
            break;
    }

    if (report)
    {
        comm::currComm->reportBestMove(bestMove);
    }

    return score;
}

int Search::aspWindows(SearchThread& thread, int depth, Move& bestMove, int prevScore)
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
        else
        {
            bestMove = thread.pv[0];
            if (searchScore >= beta)
                beta += delta;
            else
                return searchScore;
        }
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

    m_TimeMan.setLimits(limits, m_Board.sideToMove());
    m_TimeMan.startSearch();

    m_ShouldStop.store(false, std::memory_order_relaxed);

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
        if (m_TimeMan.stopHard(thread.limits))
        {
            m_ShouldStop = true;
            return alpha;
        }
    }

    if (m_ShouldStop.load(std::memory_order_relaxed))
        return alpha;

    auto& rootPly = thread.rootPly;
    auto& board = thread.board;
    auto& history = thread.history;


    alpha = std::max(alpha, -SCORE_MATE + rootPly);
    beta = std::min(beta, SCORE_MATE - rootPly);
    if (alpha >= beta)
        return alpha;

    searchPly->pvLength = 0;

    bool root = rootPly == 0;
    bool inCheck = board.checkers() != 0;

    if (eval::isImmediateDraw(board) || board.isDraw(rootPly))
        return SCORE_DRAW;

    if (rootPly >= MAX_PLY)
        return eval::evaluate(board);

    if (depth <= 0)
        return qsearch(thread, searchPly, alpha, beta);

    int ttScore;
    Move ttMove = Move();
    int ttDepth;
    TTEntry::Bound ttBound;
    bool ttHit;
    TTBucket* bucket = m_TT.probe(board.zkey(), ttHit, rootPly, ttScore, ttMove, ttDepth, ttBound);

    if (ttHit)
    {
        if (!isPV && ttDepth >= depth && (
            ttBound == TTEntry::Bound::EXACT ||
            (ttBound == TTEntry::Bound::LOWER_BOUND && ttScore >= beta) ||
            (ttBound == TTEntry::Bound::UPPER_BOUND && ttScore <= alpha)
        ))
            return ttScore;
    }
    else if (depth >= MIN_IIR_DEPTH)
        depth--;

    int staticEval = searchPly->staticEval = eval::evaluate(board);
    bool improving = !inCheck && rootPly > 1 && searchPly->staticEval > searchPly[-2].staticEval;
    BoardState state;

    if (!isPV && !inCheck)
    {
        // reverse futility pruning
        if (depth <= RFP_MAX_DEPTH && staticEval >= beta + (RFP_MARGIN - RFP_IMPROVING_FACTOR * improving) * depth)
            return staticEval;

        // null move pruning
        if (board.pliesFromNull() > 0 && staticEval >= beta)
        {
            BitBoard nonPawns = board.getColor(board.sideToMove()) ^ board.getPieces(board.sideToMove(), PieceType::PAWN);
            if ((nonPawns & (nonPawns - 1)) && depth >= NMP_MIN_DEPTH)
            {
                int r = NMP_BASE_REDUCTION + depth / NMP_DEPTH_REDUCTION;
                board.makeNullMove(state);
                rootPly++;
                int nullScore = -search(thread, depth - r, searchPly + 1, -beta, -beta + 1, false);
                rootPly--;
                board.unmakeNullMove();
                if (nullScore >= beta)
                    return nullScore;
            }
        }
    }

    MoveList moves;
    genMoves<MoveGenType::NOISY_QUIET>(board, moves);

    std::array<CHEntry*, 2> contHistEntries = {
        rootPly > 0 ? searchPly[-1].contHistEntry : nullptr,
        rootPly > 1 ? searchPly[-2].contHistEntry : nullptr
    };

    MoveOrdering ordering(
        board,
        moves,
        ttMove,
        searchPly->killers,
        contHistEntries,
        thread.history
    );

    std::array<Move, MAX_PLY + 1> childPV;
    searchPly[1].pv = childPV.data();

    searchPly->bestMove = Move();

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
            baseLMR -= moveHistory / LMR_HIST_DIVISOR;

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
                movesPlayed >= LMP_MIN_MOVES_BASE + depth * depth / (improving ? 1 : 2))
                break;

            if (!isPV &&
                depth <= MAX_SEE_PRUNE_DEPTH &&
                !board.see_margin(move, depth * (quiet ? SEE_PRUNE_MARGIN_QUIET : SEE_PRUNE_MARGIN_NOISY)))
                continue;
        }

        searchPly->contHistEntry = &history.contHistEntry(ExtMove::from(board, move));

        uint64_t nodesBefore = thread.nodes;
        board.makeMove(move, state);
        thread.nodes++;
        bool givesCheck = board.checkers() != 0;
        if (quiet)
            quietsTried.push_back(move);
        rootPly++;

        int reduction = 0;
        if (movesPlayed >= (isPV ? LMR_MIN_MOVES_PV : LMR_MIN_MOVES_NON_PV) &&
            depth >= LMR_MIN_DEPTH &&
            quietLosing &&
            !inCheck)
        {
            reduction = baseLMR;

            reduction -= isPV;
            reduction -= givesCheck;

            reduction = std::clamp(reduction, 0, depth - 2);
        }

        movesPlayed++;

        int newDepth = depth + givesCheck - 1;
        int score;
        if (movesPlayed == 1)
            score = -search(thread, newDepth, searchPly + 1, -beta, -alpha, isPV);
        else
        {
            score = -search(thread, newDepth - reduction, searchPly + 1, -(alpha + 1), -alpha, false);

            if (score > alpha && reduction > 0)
                score = -search(thread, newDepth, searchPly + 1, -(alpha + 1), -alpha, false);

            if (score > alpha && isPV)
                score = -search(thread, newDepth, searchPly + 1, -beta, -alpha, true);
        }
        rootPly--;
        board.unmakeMove(move);
        if (root && thread.isMainThread())
            m_TimeMan.updateNodes(move, thread.nodes - nodesBefore);

        searchPly->contHistEntry = nullptr;

        if (m_ShouldStop)
            return alpha;

        if (score > bestScore)
        {
            bestScore = score;

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

            if (bestScore >= beta)
            {
                if (quiet)
                {
                    storeKiller(searchPly, move);

                    int bonus = historyBonus(depth);
                    history.updateQuietStats(ExtMove::from(board, move), contHistEntries, bonus);
                    for (int j = 0; j < static_cast<int>(quietsTried.size() - 1); j++)
                    {
                        history.updateQuietStats(ExtMove::from(board, quietsTried[j]), contHistEntries, -bonus);
                    }
                }
                m_TT.store(bucket, board.zkey(), depth, rootPly, bestScore, move, TTEntry::Bound::LOWER_BOUND);
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

    int ttScore;
    Move ttMove = Move();
    int ttDepth;
    TTEntry::Bound ttBound;
    bool ttHit;
    // qsearch is always depth 0
    TTBucket* bucket = m_TT.probe(board.zkey(), ttHit, rootPly, ttScore, ttMove, ttDepth, ttBound);

    if (ttHit)
    {
        if (ttBound == TTEntry::Bound::EXACT ||
            (ttBound == TTEntry::Bound::LOWER_BOUND && ttScore >= beta) ||
            (ttBound == TTEntry::Bound::UPPER_BOUND && ttScore <= alpha)
        )
            return ttScore;
    }

    int eval = eval::evaluate(board);

    if (eval >= beta)
        return eval;
    if (eval > alpha)
        alpha = eval;

    if (rootPly >= MAX_PLY)
        return alpha;

    std::array<Move, MAX_PLY + 1> childPV;
    searchPly[1].pv = childPV.data();

    MoveList captures;
    genMoves<MoveGenType::NOISY>(board, captures);

    MoveOrdering ordering(board, captures, ttMove);

    BoardState state;
    TTEntry::Bound bound = TTEntry::Bound::UPPER_BOUND;
    searchPly->bestMove = Move();
    for (int moveIdx = 0; moveIdx < static_cast<int>(captures.size()); moveIdx++)
    {
        auto [move, moveScore, moveHistory] = ordering.selectMove(moveIdx);
        if (!board.isLegal(move))
            continue;
        if (!board.see_margin(move, 0))
            continue;
        board.makeMove(move, state);
        thread.nodes++;
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

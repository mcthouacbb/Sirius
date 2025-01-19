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

MultiArray<int, 64, 64> genLMRTable()
{
    std::array<std::array<int, 64>, 64> lmrTable = {};
    for (int d = 1; d < 64; d++)
    {
        for (int i = 1; i < 64; i++)
        {
            lmrTable[d][i] = static_cast<int>(lmrBase / 100.0 + std::log(static_cast<double>(d)) * std::log(static_cast<double>(i)) / (lmrDivisor / 100.0));
        }
    }
    return lmrTable;
}

MultiArray<int, 64, 64> lmrTable = {};

void init()
{
    lmrTable = genLMRTable();
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
        stack[i].excludedMove = Move();
        stack[i].multiExts = 0;
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

void Search::run(const SearchLimits& limits, const Board& board)
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
    thread.evalState.init(thread.board, thread.pawnTable);

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

// Aspiration Windows(~108 elo)
// Typically, the search score between depths is quite stable. So we take advantage of this
// and search on a window of (prevScore - delta, prevScore + delta), where prevScore is
// the score of the previous depth, and delta is some constant. If the search score falls
// within this window, then we can use it directly. However, if the search score is outside
// the window, then we must expand the window and search again. This process is repeated
// until we get a score which falls inside the window. The benefit of this is that a
// tighter alpha-beta window causes much more beta cutoffs So we get to search more efficiently.
// However, if the score is too unstable, then the extra researches outweigh the benefits.
// We avoid Aspiration Windows at low depths, since those scores can be quite volatile
int Search::aspWindows(SearchThread& thread, int depth, Move& bestMove, int prevScore)
{
    int delta = aspInitDelta;
    int alpha = -SCORE_MAX;
    int beta = SCORE_MAX;
    int aspDepth = depth;

    if (depth >= minAspDepth)
    {
        alpha = prevScore - delta;
        beta = prevScore + delta;
    }

    while (true)
    {
        int searchScore = search(thread, std::max(aspDepth, 1), &thread.stack[0], alpha, beta, true, false);
        if (m_ShouldStop)
            return searchScore;

        if (searchScore <= alpha)
        {
            beta = (alpha + beta) / 2;
            alpha -= delta;
            aspDepth = depth;
        }
        else
        {
            bestMove = thread.stack[0].pv[0];
            if (searchScore >= beta)
            {
                beta += delta;
                aspDepth = std::max(aspDepth - 1, depth - 5);
            }
            else
                return searchScore;
        }

        // Expand the window in an exponential fashion
        delta += delta * aspWideningFactor / 16;
    }
}

BenchData Search::benchSearch(int depth, const Board& board)
{
    SearchLimits limits = {};
    limits.maxDepth = depth;

    std::unique_ptr<SearchThread> thread = std::make_unique<SearchThread>(0, std::thread());
    thread->limits = limits;
    thread->board = board;

    m_TimeMan.setLimits(limits, m_Board.sideToMove());
    m_TimeMan.startSearch();

    m_ShouldStop.store(false, std::memory_order_relaxed);

    iterDeep(*thread, false, false);

    BenchData data = {};
    data.nodes = thread->nodes;

    return data;
}

int Search::search(SearchThread& thread, int depth, SearchStack* stack, int alpha, int beta, bool pvNode, bool cutnode)
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
    auto& history = thread.history;

    if (rootPly + 1 > thread.selDepth)
        thread.selDepth = rootPly + 1;

    // Mate Distance Pruning
    // A theoretically sound pruning technique where we prune if it is completely
    // impossible for us to find a shorter mate even if we checkmate on this move
    // This has little effect on playing strength, but massively speeds up matefinding
    alpha = std::max(alpha, -SCORE_MATE + rootPly);
    beta = std::min(beta, SCORE_MATE - rootPly);
    if (alpha >= beta)
        return alpha;

    stack->pvLength = 0;

    bool root = rootPly == 0;
    bool inCheck = board.checkers().any();
    bool excluded = stack->excludedMove != Move();

    // Detect upcoming repetitions in the search
    // More info in cuckoo.cpp
    if (!root && board.halfMoveClock() >= 3 && alpha < 0 && board.hasUpcomingRepetition(rootPly))
    {
        alpha = SCORE_DRAW;
        if (alpha >= beta)
            return alpha;
    }

    if (board.isDraw(rootPly))
        return SCORE_DRAW;

    if (rootPly >= MAX_PLY)
        return eval::evaluate(board, &thread);

    // Do a quiescence search at depth 0 to get a suitable score for the position
    if (depth <= 0)
        return qsearch(thread, stack, alpha, beta, pvNode);

    ProbedTTData ttData = {};
    bool ttHit = false;

    int rawStaticEval = SCORE_NONE;

    if (!excluded)
    {
        // Probe the transposition table
        ttHit = m_TT.probe(board.zkey(), rootPly, ttData);

        // TT Cutoffs(~101 elo)
        // If a previous search at an equal or higher depth was already done and saved in the TT,
        // Cutoff using the score of that search. The bounds checks are required because
        // most scores in the search are not exact and cannot simply be returned whenever
        // the same position is reached

        // TT Cutoffs are avoided in PV nodes because
        // 1. We want to actually do a search on PV nodes and avoid a cutoff
        // 2. Cutting off in pv nodes can create issues with repetition detection
        if (ttHit && !pvNode && ttData.depth >= depth && (
            ttData.bound == TTEntry::Bound::EXACT ||
            (ttData.bound == TTEntry::Bound::LOWER_BOUND && ttData.score >= beta) ||
            (ttData.bound == TTEntry::Bound::UPPER_BOUND && ttData.score <= alpha)
        ))
            return ttData.score;

        // Avoid using eval in check
        if (inCheck)
        {
            stack->staticEval = SCORE_NONE;
            stack->eval = SCORE_NONE;
        }
        else
        {
            rawStaticEval = ttHit ? ttData.staticEval : eval::evaluate(board, &thread);
            // Correction History(~91 elo)
            stack->staticEval = history.correctStaticEval(rawStaticEval, board);
            stack->eval = stack->staticEval;

            // Use the TT Score as a Better Eval(~8 elo)
            if (ttHit && (
                ttData.bound == TTEntry::Bound::EXACT ||
                (ttData.bound == TTEntry::Bound::LOWER_BOUND && ttData.score >= stack->eval) ||
                (ttData.bound == TTEntry::Bound::UPPER_BOUND && ttData.score <= stack->eval)
            ))
                stack->eval = ttData.score;
        }
    }

    bool ttPV = pvNode || (ttHit && ttData.pv);

    // Improving heuristic(~31 elo)
    // The improving heuristic relies is based on the idea that if the static evaluation
    // improved since the last time it was our turn, we are more confident that the position
    // is good and less confident that the position is bad. Thus, when we are improving we
    // 1. prune more aggressively on fail highs(e.g. RFP, NMP, Probcut)
    // 2. prune/reduce less agressively on fail lows(e.g. Futility Pruning, LMP, LMR)
    bool improving = !inCheck && rootPly > 1 && stack->staticEval > stack[-2].staticEval;
    bool oppWorsening =
        !inCheck && rootPly > 0 &&
        stack[-1].staticEval != SCORE_NONE && stack->staticEval > -stack[-1].staticEval + 1;

    // Clear killer moves for all child nodes
    // Killer moves eventually degrade to noise if they are left too long
    // And the search as entered too different of a branch
    stack[1].killers = {};
    Bitboard threats = board.threats();

    // Whole Node Pruning(~228 elo)
    // Prune the whole node based on certain conditions
    if (!pvNode && !inCheck && !excluded)
    {
        // Reverse Futility Pruning(~86 elo)
        // If the static evaluation is very far above beta, then it is extremely likely
        // that the search will fail high, so we prune and return a fail high. This relies
        // on the Null Move Observation, the fact that in almost every position there is at
        // least one move that is better than doing nothing
        int rfpMargin = (improving ? rfpImpMargin : rfpNonImpMargin) * depth - 20 * oppWorsening + stack[-1].histScore / rfpHistDivisor;
        if (depth <= rfpMaxDepth &&
            stack->eval >= std::max(rfpMargin, 20) + beta)
            return stack->eval;

        // Razoring(~6 elo)
        // If the static evaluation is very far below alpha, then do a quiescence search
        // on the current position. If the quiescence search also fails low, then assume
        // the current search will fail low and prune
        if (depth <= razoringMaxDepth && stack->eval <= alpha - razoringMargin * depth && alpha < 2000)
        {
            int score = qsearch(thread, stack, alpha, beta, pvNode);
            if (score <= alpha)
                return score;
        }

        // Null Move Pruning(~31 elo)
        // Based on the Null Move Observation, there is almost always at least one move that
        // is better than doing nothing. So if we make a null move, and a reduced depth search
        // fails high then it is likely that this search will also fail high
        Bitboard nonPawns = board.pieces(board.sideToMove()) ^ board.pieces(board.sideToMove(), PieceType::PAWN);
        if (board.pliesFromNull() > 0 && depth >= nmpMinDepth &&
            stack->eval >= beta && stack->staticEval >= beta + nmpEvalBaseMargin - nmpEvalDepthMargin * depth &&
            nonPawns.multiple())
        {
            int r = nmpBaseReduction + depth / nmpDepthReductionScale + std::min((stack->eval - beta) / nmpEvalReductionScale, nmpMaxEvalReduction);
            board.makeNullMove();
            rootPly++;
            int nullScore = -search(thread, depth - r, stack + 1, -beta, -beta + 1, false, !cutnode);
            rootPly--;
            board.unmakeNullMove();
            if (nullScore >= beta)
                return nullScore;
        }

        // Probcut(~3 elo)
        // If a capture with good SEE is able to fail high on a slightly reduced search,
        // then assume the search would have failed high and prune
        int probcutBeta = beta + probcutBetaMargin;
        if (depth >= probcutMinDepth &&
            !isMateScore(beta) &&
            (!ttHit || ttData.score >= probcutBeta || ttData.depth + 3 < depth))
        {
            MoveOrdering ordering(board, ttData.move, thread.history);
            ScoredMove scoredMove = {};
            int seeThreshold = probcutBeta - stack->staticEval;
            int probcutDepth = depth - probcutReduction;
            while ((scoredMove = ordering.selectMove()).score != MoveOrdering::NO_MOVE)
            {
                auto [move, moveScore] = scoredMove;
                if (!board.isLegal(move))
                    continue;
                if (!board.see(move, seeThreshold))
                    continue;

                int histScore = history.getNoisyStats(threats, ExtMove::from(board, move));

                m_TT.prefetch(board.keyAfter(move));
                stack->contHistEntry = &history.contHistEntry(ExtMove::from(board, move));
                stack->histScore = histScore;
                rootPly++;
                board.makeMove(move, thread.evalState);
                thread.nodes.fetch_add(1, std::memory_order_relaxed);

                int score = -qsearch(thread, stack + 1, -probcutBeta, -probcutBeta + 1, false);
                if (score >= probcutBeta && probcutDepth >= 0)
                    score = -search(thread, probcutDepth, stack + 1, -probcutBeta, -probcutBeta + 1, false, !cutnode);

                rootPly--;
                board.unmakeMove(thread.evalState);
                stack->contHistEntry = nullptr;
                stack->histScore = 0;

                if (score >= probcutBeta)
                {
                    m_TT.store(board.zkey(), probcutDepth + 1, rootPly, score, rawStaticEval, move, ttPV, TTEntry::Bound::LOWER_BOUND);
                    return score;
                }
            }
        }
    }

    // Internal Iterative Reductions(~8 elo)
    // If the search has no tt entry at high depth
    // 1. This search will likely take a very long time, as we cannot do tt move ordering
    // 2. This position is probably not very good anyways
    // So we reduce the depth
    if (depth >= minIIRDepth && !inCheck && !excluded && !ttHit)
        depth--;

    // continuation history(~40 elo)
    std::array<CHEntry*, 3> contHistEntries = {
        rootPly > 0 ? stack[-1].contHistEntry : nullptr,
        rootPly > 1 ? stack[-2].contHistEntry : nullptr,
        rootPly > 3 ? stack[-4].contHistEntry : nullptr
    };

    MoveOrdering ordering(
        board,
        ttData.move,
        stack->killers,
        contHistEntries,
        thread.history
    );

    stack[1].failHighCount = 0;

    stack->bestMove = Move();

    TTEntry::Bound bound = TTEntry::Bound::UPPER_BOUND;

    MoveList quietsTried;
    MoveList noisiesTried;

    int bestScore = -SCORE_MAX;
    int movesPlayed = 0;
    bool noisyTTMove = ttData.move != Move() && !moveIsQuiet(board, ttData.move);

    if (!root)
        stack->multiExts = stack[-1].multiExts;

    ScoredMove scoredMove = {};
    while ((scoredMove = ordering.selectMove()).score != MoveOrdering::NO_MOVE)
    {
        auto [move, moveScore] = scoredMove;
        if (move == stack->excludedMove)
            continue;
        if (!board.isLegal(move))
            continue;
        bool quiet = moveIsQuiet(board, move);
        bool quietLosing = moveScore < MoveOrdering::KILLER_SCORE;

        int baseLMR = lmrTable[std::min(depth, 63)][std::min(movesPlayed, 63)];
        ExtMove extMove = ExtMove::from(board, move);
        int histScore = quiet ? history.getQuietStats(threats, extMove, contHistEntries) : history.getNoisyStats(threats, extMove);
        baseLMR -= histScore / (quiet ? lmrQuietHistDivisor : lmrNoisyHistDivisor);

        // Move Loop Pruning(~184 elo)
        // Prune moves at shallow depth based on certain conditions
        if (!root && quietLosing && bestScore > -SCORE_WIN)
        {
            // Futility Pruning(~1 elo)
            // If the static evaluation is very far below alpha and this move is quiet,
            // it will likely not be able to improve the static evaluation enough
            // to raise alpha. So prune the move
            int lmrDepth = std::max(depth - baseLMR, 0);
            if (lmrDepth <= fpMaxDepth &&
                quiet &&
                !inCheck &&
                alpha < SCORE_WIN &&
                stack->eval + fpBaseMargin + fpDepthMargin * lmrDepth <= alpha)
            {
                continue;
            }

            // Late Move Pruning(~23 elo)
            // After searching a certain amount of moves at low depth, prune the rest
            // This assumes that move ordering is good and that later moves
            // are not better than early ones
            if (!pvNode &&
                !inCheck &&
                depth <= lmpMaxDepth &&
                movesPlayed >= lmpMinMovesBase + depth * depth / (improving ? 1 : 2))
                break;

            // Static Exchange Evaluation Pruning(~5 elo)
            // If the move loses a large amount of material based on SEE, then prune the move
            int seeMargin = quiet ?
                depth * seePruneMarginQuiet :
                depth * seePruneMarginNoisy - std::clamp(histScore / seeCaptHistDivisor, -seeCaptHistMax * depth, seeCaptHistMax * depth);
            if (!pvNode &&
                depth <= maxSeePruneDepth &&
                !board.see(move, seeMargin))
                continue;

            // History Pruning(~14 elo)
            // Prune moves that have a very low history at low depth
            if (quiet &&
                depth <= maxHistPruningDepth &&
                histScore < -histPruningMargin * depth)
                break;
        }

        bool doSE = !root &&
            !excluded &&
            depth >= seMinDepth &&
            ttData.move == move &&
            ttData.depth >= depth - seTTDepthMargin &&
            ttData.bound != TTEntry::Bound::UPPER_BOUND &&
            !isMateScore(ttData.score);

        int extension = 0;

        // Singular Extensions(~73 elo)
        // Extend when a move seems to be much better than all other moves in a position
        // To detect this, we search around a zero window of (ttScore - margin), with the tt move
        // excluded from this search. If this search fails low, and the tt score indicates that
        // the tt move failed high or is exact, then the tt move is considered singular,
        // and we extend it
        if (doSE)
        {
            int sBeta = std::max(-SCORE_MATE, ttData.score - sBetaScale * depth / 16);
            int sDepth = (depth - 1) / 2;
            stack->excludedMove = ttData.move;

            int score = search(thread, sDepth, stack, sBeta - 1, sBeta, false, cutnode);

            stack->excludedMove = Move();

            if (score < sBeta)
            {
                // Double extensions
                // If the score is below singular beta by a large margin, then the tt move is very singular
                // and we extend by 2 plies
                if (!pvNode && stack->multiExts < maxMultiExts && score < sBeta - doubleExtMargin)
                    extension = 2;
                else
                    extension = 1;
            }

            // Multicut
            // If the singular search failed high and singular beta is above beta
            // There are likely multiple moves in this position that would cause a fail high
            else if (sBeta >= beta)
                return sBeta;

            // Negative Extensions
            // We can't do multicut, but the singular search indicates that other moves are
            // potentially as good or better than the tt move. So we reduce the depth
            // of the tt move in favor of other moves
            else if (ttData.score >= beta)
                extension = -2 + pvNode;
        }

        stack->multiExts += extension >= 2;

        m_TT.prefetch(board.keyAfter(move));
        stack->contHistEntry = &history.contHistEntry(extMove);
        stack->histScore = histScore;

        uint64_t nodesBefore = thread.nodes;
        board.makeMove(move, thread.evalState);
        thread.nodes.fetch_add(1, std::memory_order_relaxed);
        bool givesCheck = board.checkers().any();
        // check extensions(~13 elo)
        if (!doSE && givesCheck)
            extension = 1;

        if (quiet)
            quietsTried.push_back(move);
        else
            noisiesTried.push_back(move);

        rootPly++;
        movesPlayed++;

        int newDepth = depth + extension - 1;
        int score = 0;

        // Late Move Reductions(~111 elo)
        // If move ordering is good, then later moves are likely much worse than earlier moves
        // So we search these moves at a reduced depth, and save a ton of time doing so
        // If the reduced search is able to raise alpha against our expectations
        // We do a research at full depth
        if (movesPlayed >= (pvNode ? lmrMinMovesPv : lmrMinMovesNonPv) &&
            depth >= lmrMinDepth &&
            quietLosing)
        {
            int reduction = baseLMR;

            reduction += !improving;
            reduction += noisyTTMove;
            reduction -= ttPV;
            reduction -= givesCheck;
            reduction -= inCheck;
            reduction -= std::abs(stack->eval - rawStaticEval) > lmrCorrplexityMargin;
            reduction += cutnode;
            reduction += stack[1].failHighCount >= static_cast<uint32_t>(lmrFailHighCountMargin);

            int reduced = std::min(std::max(newDepth - reduction, 1), newDepth);
            score = -search(thread, reduced, stack + 1, -alpha - 1, -alpha, false, true);
            if (score > alpha && reduced < newDepth)
            {
                bool doDeeper = score > bestScore + doDeeperMarginBase + doDeeperMarginDepth * newDepth / 16;
                bool doShallower = score < bestScore + doShallowerMargin;
                newDepth += doDeeper - doShallower;
                score = -search(thread, newDepth, stack + 1, -alpha - 1, -alpha, false, !cutnode);

                if (quiet && (score <= alpha || score >= beta))
                {
                    int bonus = score >= beta ? historyBonus(depth) : -historyMalus(depth);
                    history.updateContHist(extMove, contHistEntries, bonus);
                }
            }
        }
        else if (!pvNode || movesPlayed > 1)
            score = -search(thread, newDepth, stack + 1, -alpha - 1, -alpha, false, !cutnode);

        if (pvNode && (movesPlayed == 1 || score > alpha))
            score = -search(thread, newDepth, stack + 1, -beta, -alpha, true, false);

        rootPly--;
        board.unmakeMove(thread.evalState);
        if (root && thread.isMainThread())
            m_TimeMan.updateNodes(move, thread.nodes - nodesBefore);

        stack->contHistEntry = nullptr;
        stack->histScore = 0;

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
                if (pvNode)
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
                // Killer Moves(~6 elo)
                // If a quiet move causes a beta cutoff in this node
                // It will probably also work in other nodes at the same ply
                if (quiet)
                {
                    if (stack->killers[0] != move)
                    {
                        stack->killers[1] = stack->killers[0];
                        stack->killers[0] = move;
                    }
                }

                // history(~527 elo)
                // Updates the history tables
                // The move which failed high gets a bonus to its history
                // and moves which were unable to cause a cutoff get a penalty
                int histDepth = depth + (bestScore > beta + histBetaMargin);
                int bonus = historyBonus(histDepth);
                int malus = historyMalus(histDepth);
                if (quiet)
                {
                    history.updateQuietStats(threats, ExtMove::from(board, move), contHistEntries, bonus);
                    // Only punish quiets if the cutoff move was quiet
                    for (Move quietMove : quietsTried)
                    {
                        if (quietMove != move)
                            history.updateQuietStats(threats, ExtMove::from(board, quietMove), contHistEntries, -malus);
                    }
                }
                else
                {
                    history.updateNoisyStats(threats, ExtMove::from(board, move), bonus);
                }

                // unconditionally punish capture moves
                for (Move noisyMove : noisiesTried)
                {
                    if (noisyMove != move)
                        history.updateNoisyStats(threats, ExtMove::from(board, noisyMove), -malus);
                }
                bound = TTEntry::Bound::LOWER_BOUND;
                break;
            }
        }
    }

    // check for mates
    if (movesPlayed == 0)
    {
        if (inCheck)
            return -SCORE_MATE + rootPly;
        return SCORE_DRAW;
    }

    // Avoid updating correction history and the transposition table in singular searches
    // In a singular search, we are specifically avoiding the likely best move
    // so the score and best move of the singular search are useless for normal searches
    if (!excluded)
    {
        // update correction histories
        if (!inCheck && (stack->bestMove == Move() || moveIsQuiet(board, stack->bestMove)) &&
            !(bound == TTEntry::Bound::LOWER_BOUND && stack->staticEval >= bestScore) &&
            !(bound == TTEntry::Bound::UPPER_BOUND && stack->staticEval <= bestScore))
            history.updateCorrHist(bestScore - stack->staticEval, depth, board);

        // Store the search results to the TT
        m_TT.store(board.zkey(), depth, rootPly, bestScore, rawStaticEval, stack->bestMove, ttPV, bound);
    }

    return bestScore;
}

// Quiescence Search(~187 elo)
// The evaluation function does not work well when the position has lots of good captures
// So, a quiescence search is used to evaluate nodes near the leaves of the search tree.
// A qsearch is a search that only looks to resolve captures and evaluate quiet positions
// to get a more accurate and stable score
int Search::qsearch(SearchThread& thread, SearchStack* stack, int alpha, int beta, bool pvNode)
{
    auto& rootPly = thread.rootPly;
    auto& board = thread.board;
    auto& history = thread.history;

    stack->pvLength = 0;
    if (board.isInsufMaterialDraw())
        return SCORE_DRAW;

    if (rootPly + 1 > thread.selDepth)
        thread.selDepth = rootPly + 1;

    ProbedTTData ttData = {};
    bool ttHit = m_TT.probe(board.zkey(), rootPly, ttData);
    bool ttPV = pvNode || (ttHit && ttData.pv);

    // TT Cutoffs(~101 elo)
    if (ttHit && !pvNode && (
        ttData.bound == TTEntry::Bound::EXACT ||
        (ttData.bound == TTEntry::Bound::LOWER_BOUND && ttData.score >= beta) ||
        (ttData.bound == TTEntry::Bound::UPPER_BOUND && ttData.score <= alpha)
    ))
        return ttData.score;

    bool inCheck = board.checkers().any();
    int rawStaticEval = SCORE_NONE;
    if (inCheck)
    {
        stack->staticEval = SCORE_NONE;
        stack->eval = SCORE_NONE;
    }
    else
    {
        rawStaticEval = ttHit ? ttData.staticEval : eval::evaluate(board, &thread);
        // Correction History(~91 elo)
        stack->staticEval = inCheck ? SCORE_NONE : thread.history.correctStaticEval(rawStaticEval, board);

        // Use the TT Score as a Better Eval(~8 elo)
        stack->eval = stack->staticEval;
        if (!inCheck && ttHit && (
            ttData.bound == TTEntry::Bound::EXACT ||
            (ttData.bound == TTEntry::Bound::LOWER_BOUND && ttData.score >= stack->eval) ||
            (ttData.bound == TTEntry::Bound::UPPER_BOUND && ttData.score <= stack->eval)
        ))
            stack->eval = ttData.score;
    }

    if (stack->eval >= beta)
        return stack->eval;
    if (stack->eval > alpha)
        alpha = stack->eval;

    int bestScore = inCheck ? -SCORE_MAX : stack->eval;
    int futility = inCheck ? -SCORE_MAX : stack->eval + qsFpMargin;

    if (rootPly >= MAX_PLY)
        return alpha;

    std::array<CHEntry*, 3> contHistEntries = {
        rootPly > 0 ? stack[-1].contHistEntry : nullptr,
        rootPly > 1 ? stack[-2].contHistEntry : nullptr,
        rootPly > 3 ? stack[-4].contHistEntry : nullptr
    };

    MoveOrdering ordering = [&]()
    {
        // Generate all moves when in check, instead of just captures
        // This helps the quiescence search resolve checks more accurately
        if (inCheck)
            return MoveOrdering(board, ttData.move, stack->killers, contHistEntries, history);
        else
            return MoveOrdering(board, ttData.move, history);
    }();

    TTEntry::Bound bound = TTEntry::Bound::UPPER_BOUND;
    stack->bestMove = Move();
    int movesPlayed = 0;

    ScoredMove scoredMove = {};
    while ((scoredMove = ordering.selectMove()).score != MoveOrdering::NO_MOVE)
    {
        // quiescence search pruning(~55 elo)

        // QSearch LMP
        // Capture move ordering is usually very good so late moves are likely not very good
        if (!inCheck && movesPlayed >= 2)
            break;
        auto [move, moveScore] = scoredMove;
        if (!board.isLegal(move))
            continue;

        // QSearch SEE Pruning
        // If this move loses material according to SEE
        // It is likely not going to cause a cutoff or raise alpha
        if (bestScore > -SCORE_WIN && !board.see(move, 0))
            continue;

        // QSearch Futility Pruning
        // If the static evaluation is far below alpha, and this move does not win material
        // It is unlikely that it will be able to raise alpha or cause a cutoff
        if (!inCheck && futility <= alpha && !board.see(move, 1))
        {
            bestScore = std::max(bestScore, futility);
            continue;
        }
        movesPlayed++;
        stack->contHistEntry = &history.contHistEntry(ExtMove::from(board, move));
        board.makeMove(move, thread.evalState);
        thread.nodes.fetch_add(1, std::memory_order_relaxed);
        rootPly++;

        int score = -qsearch(thread, stack + 1, -beta, -alpha, pvNode);

        board.unmakeMove(thread.evalState);
        stack->contHistEntry = nullptr;
        rootPly--;

        if (score > bestScore)
        {
            bestScore = score;

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

            if (bestScore >= beta)
            {
                bound = TTEntry::Bound::LOWER_BOUND;
                break;
            }
        }
    }

    // Check for mate
    // We can only consider checkmates, since, when not in check, we only generate captures
    // Thus, we have no idea whether there are any legal quiet moves
    if (inCheck && movesPlayed == 0)
        return -SCORE_MATE + rootPly;

    m_TT.store(board.zkey(), 0, rootPly, bestScore, rawStaticEval, stack->bestMove, ttPV, bound);

    return bestScore;
}


}

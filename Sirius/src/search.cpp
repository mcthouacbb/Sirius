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

MultiArray<int, 64, 64> genLMRTable()
{
    std::array<std::array<int, 64>, 64> lmrTable = {};
    for (int d = 1; d < 64; d++)
    {
        for (int i = 1; i < 64; i++)
        {
            double base = static_cast<double>(lmrBase) / 100.0;
            double divisor = static_cast<double>(lmrDivisor) / 100.0;
            lmrTable[d][i] = static_cast<int>(
                base + std::log(static_cast<double>(d)) * std::log(static_cast<double>(i)) / divisor);
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
        stack[i].playedMove = Move::nullmove();
        stack[i].movedPiece = Piece::NONE;
        stack[i].killers[0] = stack[i].killers[1] = Move::nullmove();
        stack[i].excludedMove = Move::nullmove();
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
    : m_ShouldStop(false), m_TT(64)
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
    m_TT.reset(m_Threads.size());
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
                iterDeep(thread, thread.isMainThread(), true);
                break;
            case WakeFlag::NONE:
                // unreachable;
                break;
        }
    }
}

void Search::makeMove(SearchThread& thread, SearchStack* stack, Move move, int histScore)
{
    stack->contHistEntry = &thread.history.contHistEntry(thread.board, move);
    stack->histScore = histScore;
    stack->playedMove = move;
    stack->movedPiece = movingPiece(thread.board, move);
    stack->contCorrEntry = &thread.history.contCorrEntry(thread.board, move);

    thread.rootPly++;
    thread.board.makeMove(move, thread.evalState);
    thread.nodes.fetch_add(1, std::memory_order_relaxed);
}

void Search::unmakeMove(SearchThread& thread, SearchStack* stack)
{
    thread.rootPly--;
    thread.board.unmakeMove(thread.evalState);

    stack->contHistEntry = nullptr;
    stack->histScore = 0;
    stack->playedMove = Move::nullmove();
    stack->movedPiece = Piece::NONE;
    stack->contCorrEntry = nullptr;
}

void Search::makeNullMove(SearchThread& thread, SearchStack* stack)
{
    stack->contCorrEntry = &thread.history.contCorrEntry(thread.board, Move::nullmove());
    thread.board.makeNullMove();
    thread.rootPly++;
}

void Search::unmakeNullMove(SearchThread& thread, SearchStack* stack)
{
    thread.rootPly--;
    thread.board.unmakeNullMove();
    stack->contCorrEntry = nullptr;
}

int Search::iterDeep(SearchThread& thread, bool report, bool normalSearch)
{
    int maxDepth = std::min(thread.limits.maxDepth, MAX_PLY - 1);
    int score = 0;
    Move bestMove = Move::nullmove();

    thread.reset();
    thread.evalState.init(thread.board, thread.pawnTable);
    thread.initRootMoves();

    report = report && normalSearch;

    for (int depth = 1; depth <= maxDepth; depth++)
    {
        thread.rootDepth = depth;
        thread.selDepth = 0;
        int searchScore = aspWindows(thread, depth, bestMove, score);
        thread.sortRootMoves();
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
            info.hashfull = m_TT.hashfull();
            info.time = m_TimeMan.elapsed();
            info.pvBegin = thread.stack[0].pv.data();
            info.pvEnd = thread.stack[0].pv.data() + thread.stack[0].pvLength;
            info.score = searchScore;
            uci::uci->reportSearchInfo(info);
        }
        uint64_t bmNodes = thread.findRootMove(bestMove).nodes;
        if (thread.isMainThread() && m_TimeMan.stopSoft(bestMove, bmNodes, thread.nodes, thread.limits))
            break;
    }

    if (thread.isMainThread())
        m_ShouldStop.store(true, std::memory_order_relaxed);

    if (report)
        uci::uci->reportBestMove(bestMove);

    return score;
}

// Aspiration windows(~108 elo)
int Search::aspWindows(SearchThread& thread, int depth, Move& bestMove, int prevScore)
{
    int delta = aspInitDelta + prevScore * prevScore / 16384;
    int alpha = -SCORE_MAX;
    int beta = SCORE_MAX;
    int aspDepth = depth;

    if (depth >= minAspDepth)
    {
        alpha = std::max(prevScore - delta, -SCORE_MAX);
        beta = std::min(prevScore + delta, SCORE_MAX);
    }

    while (true)
    {
        int searchScore =
            search(thread, std::max(aspDepth, 1), &thread.stack[0], alpha, beta, true, false);
        if (m_ShouldStop)
            return searchScore;

        if (searchScore <= alpha)
        {
            beta = (alpha + beta) / 2;
            alpha = std::max(alpha - delta, -SCORE_MAX);
            aspDepth = depth;
        }
        else
        {
            bestMove = thread.stack[0].pv[0];
            if (searchScore >= beta)
            {
                beta = std::min(beta + delta, SCORE_MAX);
                aspDepth = std::max(aspDepth - 1, depth - 5);
            }
            else
                return searchScore;
        }
        delta += delta * aspWideningFactor / 256;
    }
}

BenchData Search::benchSearch(int depth, const Board& board)
{
    SearchLimits limits = {};
    limits.maxDepth = depth;

    std::unique_ptr<SearchThread> thread = std::make_unique<SearchThread>(0, std::thread());
    thread->limits = limits;
    thread->board = board;

    m_TimeMan.setLimits(limits, board.sideToMove());
    m_TimeMan.startSearch();

    m_ShouldStop.store(false, std::memory_order_relaxed);

    iterDeep(*thread, false, false);

    BenchData data = {};
    data.nodes = thread->nodes;

    return data;
}

int Search::search(SearchThread& thread, int depth, SearchStack* stack, int alpha, int beta,
    bool pvNode, bool cutnode)
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

    alpha = std::max(alpha, -SCORE_MATE + rootPly);
    beta = std::min(beta, SCORE_MATE - rootPly);
    if (alpha >= beta)
        return alpha;

    stack->pvLength = 0;

    bool root = rootPly == 0;
    bool inCheck = board.checkers().any();
    bool excluded = stack->excludedMove != Move::nullmove();

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

    if (depth <= 0)
        return qsearch(thread, stack, alpha, beta, pvNode);

    ProbedTTData ttData = {};
    bool ttHit = false;

    int rawStaticEval = SCORE_NONE;

    if (!excluded)
    {
        ttHit = m_TT.probe(board.zkey(), rootPly, ttData);

        // TT Cutoffs(~101 elo)
        if (ttHit && !pvNode && ttData.depth >= depth
            && (ttData.bound == TTEntry::Bound::EXACT
                || (ttData.bound == TTEntry::Bound::LOWER_BOUND && ttData.score >= beta)
                || (ttData.bound == TTEntry::Bound::UPPER_BOUND && ttData.score <= alpha)))
            return ttData.score;

        if (inCheck)
        {
            stack->staticEval = SCORE_NONE;
            stack->eval = SCORE_NONE;
        }
        else
        {
            rawStaticEval = ttHit ? ttData.staticEval : eval::evaluate(board, &thread);
            // Correction history(~104 elo)
            stack->staticEval = history.correctStaticEval(board, rawStaticEval, stack, rootPly);
            stack->eval = stack->staticEval;
            // use tt score as a better eval(~8 elo)
            if (ttHit
                && (ttData.bound == TTEntry::Bound::EXACT
                    || (ttData.bound == TTEntry::Bound::LOWER_BOUND && ttData.score >= stack->eval)
                    || (ttData.bound == TTEntry::Bound::UPPER_BOUND && ttData.score <= stack->eval)))
                stack->eval = ttData.score;
        }
    }

    bool ttPV = pvNode || (ttHit && ttData.pv);
    // Improving heuristic(~31 elo)
    bool improving = [&]()
    {
        if (inCheck)
            return false;
        if (rootPly > 1 && (stack - 2)->staticEval != SCORE_NONE)
            return stack->staticEval > (stack - 2)->staticEval;
        if (rootPly > 3 && (stack - 4)->staticEval != SCORE_NONE)
            return stack->staticEval > (stack - 4)->staticEval;
        return true;
    }();
    bool oppWorsening = !inCheck && rootPly > 0 && (stack - 1)->staticEval != SCORE_NONE
        && stack->staticEval > -(stack - 1)->staticEval + 1;

    (stack + 1)->killers = {};
    Bitboard threats = board.threats();

    // whole node pruning(~228 elo)
    if (!pvNode && !inCheck && !excluded)
    {
        // reverse futility pruning(~86 elo)
        int rfpMargin = (improving ? rfpImpMargin : rfpNonImpMargin) * depth
            - rfpOppWorsening * oppWorsening + (stack - 1)->histScore / rfpHistDivisor;
        if (depth <= rfpMaxDepth && stack->eval >= std::max(rfpMargin, 20) + beta)
            return stack->eval;

        // razoring(~6 elo)
        if (depth <= razoringMaxDepth && stack->eval <= alpha - razoringMargin * depth && alpha < 2000)
        {
            int score = qsearch(thread, stack, alpha, beta, pvNode);
            if (score <= alpha)
                return score;
        }

        // null move pruning(~31 elo)
        Bitboard nonPawns =
            board.pieces(board.sideToMove()) ^ board.pieces(board.sideToMove(), PieceType::PAWN);
        if (board.pliesFromNull() > 0 && rootPly >= thread.nmpMinPly && depth >= nmpMinDepth
            && stack->eval >= beta
            && stack->staticEval >= beta + nmpEvalBaseMargin - nmpEvalDepthMargin * depth
            && nonPawns.multiple())
        {
            int r = (nmpBaseReduction + depth * nmpDepthReductionScale) / 256
                + std::min((stack->eval - beta) / nmpEvalReductionScale, nmpMaxEvalReduction);
            makeNullMove(thread, stack);
            int nullScore = -search(thread, depth - r, stack + 1, -beta, -beta + 1, false, !cutnode);
            unmakeNullMove(thread, stack);
            if (nullScore >= beta)
            {
                if (depth <= 15 || thread.nmpMinPly > 0)
                    return isMateScore(nullScore) ? beta : nullScore;

                thread.nmpMinPly = rootPly + (depth - r) * 3 / 4;
                int verifScore = search(thread, depth - r, stack + 1, beta - 1, beta, false, true);
                thread.nmpMinPly = 0;

                if (verifScore >= beta)
                    return verifScore;
            }
        }

        // probcut(~3 elo)
        int probcutBeta = beta + probcutBetaMargin;
        if (depth >= probcutMinDepth && !isMateScore(beta)
            && (!ttHit || ttData.score >= probcutBeta || ttData.depth + 3 < depth))
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

                m_TT.prefetch(board.keyAfter(move));

                makeMove(thread, stack, move, history.getNoisyStats(board, move));

                int score = -qsearch(thread, stack + 1, -probcutBeta, -probcutBeta + 1, false);
                if (score >= probcutBeta && probcutDepth >= 0)
                    score = -search(thread, probcutDepth, stack + 1, -probcutBeta, -probcutBeta + 1,
                        false, !cutnode);

                unmakeMove(thread, stack);

                if (m_ShouldStop)
                    return alpha;

                if (score >= probcutBeta)
                {
                    m_TT.store(board.zkey(), rootPly, probcutDepth + 1, score, rawStaticEval, move,
                        ttPV, TTEntry::Bound::LOWER_BOUND);
                    return score;
                }
            }
        }
    }

    // internal iterative reductions(~8 elo)
    if (depth >= minIIRDepth && !inCheck && !excluded
        && (!ttHit || (ttData.move != Move::nullmove() && ttData.depth <= depth - 5)))
        depth--;

    MoveOrdering ordering(board, ttData.move, stack->killers, stack, rootPly, thread.history);

    (stack + 1)->failHighCount = 0;

    Move bestMove = Move::nullmove();

    TTEntry::Bound bound = TTEntry::Bound::UPPER_BOUND;

    MoveList quietsTried;
    MoveList noisiesTried;

    int bestScore = -SCORE_MAX;
    int movesPlayed = 0;
    bool noisyTTMove = ttData.move != Move::nullmove() && !moveIsQuiet(board, ttData.move);

    ScoredMove scoredMove = {};
    while ((scoredMove = ordering.selectMove()).score != MoveOrdering::NO_MOVE)
    {
        auto [move, moveScore] = scoredMove;
        if (move == stack->excludedMove)
            continue;
        if (!board.isLegal(move))
            continue;

        bool quiet = moveIsQuiet(board, move);
        Piece movedPiece = movingPiece(board, move);
        int baseLMR = lmrTable[std::min(depth, 63)][std::min(movesPlayed, 63)];
        int histScore = quiet ? history.getQuietStats(move, threats, movedPiece, stack, rootPly)
                              : history.getNoisyStats(board, move);
        baseLMR -= histScore / (quiet ? lmrQuietHistDivisor : lmrNoisyHistDivisor);

        // move loop pruning(~184 elo)
        if (!root && moveScore < MoveOrdering::SECOND_KILLER_SCORE && bestScore > -SCORE_WIN)
        {
            // futility pruning(~1 elo)
            int lmrDepth = std::max(depth - baseLMR, 0);
            int fpMargin =
                std::max(fpBaseMargin + fpDepthMargin * lmrDepth + histScore / fpHistDivisor, 20);
            if (lmrDepth <= fpMaxDepth && quiet && !inCheck && alpha < SCORE_WIN
                && stack->staticEval + fpMargin <= alpha)
            {
                continue;
            }

            // noisy futility pruning
            fpMargin = std::max(
                noisyFPBaseMargin + noisyFpDepthMargin * depth + histScore / noisyFpHistDivisor, 20);
            if (depth <= noisyFpMaxDepth && !quiet && !inCheck && alpha < SCORE_WIN
                && stack->staticEval + fpMargin <= alpha)
            {
                if (!isMateScore(bestScore) && bestScore <= stack->staticEval + fpMargin)
                    bestScore = stack->staticEval + fpMargin;
                break;
            }

            // late move pruning(~23 elo)
            int lmpMargin = improving ? (lmpImpBase + depth * depth * lmpImpDepth) / 256
                                      : (lmpNonImpBase + depth * depth * lmpNonImpDepth) / 256;
            if (!inCheck && movesPlayed >= lmpMargin)
                break;

            // static exchange evaluation pruning(~5 elo)
            int seeMargin = quiet ? depth * seePruneMarginQuiet : depth * seePruneMarginNoisy;
            if (!quiet)
            {
                int max = seeCaptHistMax * depth;
                seeMargin -= std::clamp(histScore / seeCaptHistDivisor, -max, max);
            }
            if (!board.see(move, seeMargin))
                continue;

            // history pruning(~14 elo)
            if (quiet && depth <= maxHistPruningDepth && histScore < -histPruningMargin * depth)
                break;
        }

        bool doSE = !root && rootPly < 2 * thread.rootDepth && !excluded && depth >= seMinDepth
            && ttData.move == move && ttData.depth >= depth - seTTDepthMargin
            && ttData.bound != TTEntry::Bound::UPPER_BOUND && !isMateScore(ttData.score);

        int extension = 0;

        // singular extensions(~81 elo STC, ~91 elo LTC)
        if (doSE)
        {
            int sBeta = std::max(-SCORE_MATE, ttData.score - sBetaScale * depth / 64);
            int sDepth = (depth - 1) / 2;
            stack->excludedMove = ttData.move;

            int score = search(thread, sDepth, stack, sBeta - 1, sBeta, false, cutnode);

            stack->excludedMove = Move::nullmove();

            if (score < sBeta)
            {
                if (!pvNode && score < sBeta - doubleExtMargin)
                    extension = 2;
                else
                    extension = 1;
            }
            else if (sBeta >= beta)
                return sBeta;
            else if (ttData.score >= beta)
                extension = -2 + pvNode;
            else if (ttData.score <= alpha && cutnode)
                extension = -1;
        }

        m_TT.prefetch(board.keyAfter(move));
        uint64_t nodesBefore = thread.nodes;

        makeMove(thread, stack, move, histScore);
        movesPlayed++;

        bool givesCheck = board.checkers().any();
        // check extensions(~13 elo)
        if (!doSE && givesCheck)
            extension = 1;

        if (quiet)
            quietsTried.push_back(move);
        else
            noisiesTried.push_back(move);

        int newDepth = depth + extension - 1;
        int score = 0;

        // late move reductions(~111 elo)
        if (movesPlayed >= (pvNode ? lmrMinMovesPv : lmrMinMovesNonPv) && depth >= lmrMinDepth
            && moveScore <= MoveOrdering::FIRST_KILLER_SCORE)
        {
            int reduction = baseLMR;

            reduction += !improving;
            reduction += noisyTTMove;
            reduction -= ttPV;
            reduction -= givesCheck;
            reduction -= inCheck;
            reduction -= std::abs(stack->staticEval - rawStaticEval) > lmrCorrplexityMargin;
            reduction += cutnode;
            reduction += (stack + 1)->failHighCount >= static_cast<uint32_t>(lmrFailHighCountMargin);

            int reduced = std::min(std::max(newDepth - reduction, 1), newDepth);
            score = -search(thread, reduced, stack + 1, -alpha - 1, -alpha, false, true);
            if (score > alpha && reduced < newDepth)
            {
                bool doDeeper =
                    score > bestScore + doDeeperMarginBase + doDeeperMarginDepth * newDepth / 64;
                bool doShallower = score < bestScore + doShallowerMargin;
                newDepth += doDeeper - doShallower;
                score = -search(thread, newDepth, stack + 1, -alpha - 1, -alpha, false, !cutnode);

                if (quiet && (score <= alpha || score >= beta))
                {
                    int bonus = score >= beta ? historyBonus(depth) : -historyMalus(depth);
                    history.updateContHist(move, movedPiece, stack, rootPly - 1, bonus);
                }
            }
        }
        else if (!pvNode || movesPlayed > 1)
            score = -search(thread, newDepth, stack + 1, -alpha - 1, -alpha, false, !cutnode);

        if (pvNode && (movesPlayed == 1 || score > alpha))
            score = -search(thread, newDepth, stack + 1, -beta, -alpha, true, false);

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
                rootMove.score = score;
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
                bound = TTEntry::Bound::EXACT;
                alpha = bestScore;
                bestMove = move;
                if (pvNode)
                {
                    stack->pv[0] = move;
                    stack->pvLength = (stack + 1)->pvLength + 1;
                    for (int i = 0; i < (stack + 1)->pvLength; i++)
                        stack->pv[i + 1] = (stack + 1)->pv[i];
                }
            }

            if (bestScore >= beta)
            {
                stack->failHighCount++;
                // killer moves(~6 elo)
                if (quiet)
                {
                    if (stack->killers[0] != move)
                    {
                        stack->killers[1] = stack->killers[0];
                        stack->killers[0] = move;
                    }
                }

                // history(~527 elo)
                int histDepth = depth + (bestScore > beta + histBetaMargin);
                int bonus = historyBonus(histDepth);
                int malus = historyMalus(histDepth);
                if (quiet)
                {
                    history.updateQuietStats(board, move, stack, rootPly, bonus);
                    for (Move quietMove : quietsTried)
                    {
                        if (quietMove != move)
                            history.updateQuietStats(board, quietMove, stack, rootPly, -malus);
                    }
                }
                else
                {
                    history.updateNoisyStats(board, move, bonus);
                }

                for (Move noisyMove : noisiesTried)
                {
                    if (noisyMove != move)
                        history.updateNoisyStats(board, noisyMove, -malus);
                }
                bound = TTEntry::Bound::LOWER_BOUND;
                break;
            }
        }
    }

    if (movesPlayed == 0)
    {
        if (excluded)
            return alpha;
        if (inCheck)
            return -SCORE_MATE + rootPly;
        return SCORE_DRAW;
    }

    if (!excluded)
    {
        if (!inCheck && (bestMove == Move::nullmove() || moveIsQuiet(board, bestMove))
            && !(bound == TTEntry::Bound::LOWER_BOUND && stack->staticEval >= bestScore)
            && !(bound == TTEntry::Bound::UPPER_BOUND && stack->staticEval <= bestScore))
            history.updateCorrHist(board, bestScore - stack->staticEval, depth, stack, rootPly);

        m_TT.store(board.zkey(), rootPly, depth, bestScore, rawStaticEval, bestMove, ttPV, bound);
    }

    return bestScore;
}

// quiescence search(~187 elo)
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

    // tt cutoffs(~101 elo)
    if (ttHit && !pvNode
        && (ttData.bound == TTEntry::Bound::EXACT
            || (ttData.bound == TTEntry::Bound::LOWER_BOUND && ttData.score >= beta)
            || (ttData.bound == TTEntry::Bound::UPPER_BOUND && ttData.score <= alpha)))
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
        // Correction history(~104 elo)
        stack->staticEval = inCheck
            ? SCORE_NONE
            : thread.history.correctStaticEval(board, rawStaticEval, stack, rootPly);

        // use tt score as a better eval(~8 elo)
        stack->eval = stack->staticEval;
        if (ttHit
            && (ttData.bound == TTEntry::Bound::EXACT
                || (ttData.bound == TTEntry::Bound::LOWER_BOUND && ttData.score >= stack->eval)
                || (ttData.bound == TTEntry::Bound::UPPER_BOUND && ttData.score <= stack->eval)))
            stack->eval = ttData.score;
    }

    if (stack->eval >= beta)
    {
        m_TT.store(board.zkey(), rootPly, 0, stack->eval, rawStaticEval, Move::nullmove(), ttPV,
            TTEntry::Bound::LOWER_BOUND);
        return stack->eval;
    }
    if (stack->eval > alpha)
        alpha = stack->eval;

    int bestScore = inCheck ? -SCORE_MAX : stack->eval;
    int futility = inCheck ? -SCORE_MAX : stack->eval + qsFpMargin;

    if (rootPly >= MAX_PLY)
        return alpha;

    MoveOrdering ordering = [&]()
    {
        if (inCheck)
            return MoveOrdering(board, ttData.move, stack->killers, stack, rootPly, history);
        else
            return MoveOrdering(board, ttData.move, history);
    }();

    TTEntry::Bound bound = TTEntry::Bound::UPPER_BOUND;
    Move bestMove = Move::nullmove();
    int movesPlayed = 0;

    ScoredMove scoredMove = {};
    while ((scoredMove = ordering.selectMove()).score != MoveOrdering::NO_MOVE)
    {
        // quiescence search pruning(~55 elo)
        if (!inCheck && movesPlayed >= 2)
            break;
        auto [move, moveScore] = scoredMove;
        if (!board.isLegal(move))
            continue;
        if (bestScore > -SCORE_WIN && !board.see(move, 0))
            continue;
        if (!inCheck && futility <= alpha && !board.see(move, 1))
        {
            bestScore = std::max(bestScore, futility);
            continue;
        }

        makeMove(thread, stack, move, 0);
        movesPlayed++;

        int score = -qsearch(thread, stack + 1, -beta, -alpha, pvNode);

        unmakeMove(thread, stack);

        if (score > bestScore)
        {
            bestScore = score;

            if (bestScore > alpha)
            {
                bestMove = move;

                stack->pvLength = (stack + 1)->pvLength + 1;
                stack->pv[0] = move;
                for (int i = 0; i < (stack + 1)->pvLength; i++)
                    stack->pv[i + 1] = (stack + 1)->pv[i];

                alpha = bestScore;
            }

            if (bestScore >= beta)
            {
                bound = TTEntry::Bound::LOWER_BOUND;
                break;
            }
        }

        // Obsidian idea
        if (moveIsQuiet(board, move) && inCheck && bestScore > -SCORE_WIN)
            break;
    }

    if (inCheck && movesPlayed == 0)
        return -SCORE_MATE + rootPly;

    m_TT.store(board.zkey(), rootPly, 0, bestScore, rawStaticEval, bestMove, ttPV, bound);

    return bestScore;
}

}

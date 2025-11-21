#pragma once

#include "board.h"
#include "defs.h"
#include "eval/eval_state.h"
#include "eval/pawn_table.h"
#include "history.h"
#include "time_man.h"
#include "tt.h"

#include <array>
#include <atomic>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>
#include <vector>

struct SearchStack
{
    std::array<Move, MAX_PLY + 1> pv;
    i32 pvLength;

    Move playedMove;
    Piece movedPiece;
    Move excludedMove;

    std::array<Move, 2> killers;

    ContCorrEntry* contCorrEntry;
    CHEntry* contHistEntry;
    i32 histScore;

    i32 staticEval;
    i32 eval;

    u32 failHighCount;
};

struct SearchInfo
{
    i32 depth;
    i32 selDepth;
    i32 hashfull;
    u64 nodes;
    Duration time;
    std::vector<Move> pv;
    i32 score;
    bool lowerbound;
    bool upperbound;
};

struct BenchData
{
    u64 nodes;
};

namespace search
{

void init();

enum class WakeFlag
{
    NONE,
    SEARCH,
    QUIT
};

struct RootMove
{
    Move move = Move::nullmove();
    u64 nodes = 0;
    i32 score = SCORE_NONE;
    i32 previousScore = SCORE_NONE;
    i32 displayScore = SCORE_NONE;
    i32 selDepth = 0;
    bool lowerbound = false;
    bool upperbound = false;
    std::vector<Move> pv;

    RootMove(Move move);
};

inline RootMove::RootMove(Move move)
    : move(move)
{
}

struct SearchThread
{
    SearchThread(u32 id, std::thread&& thread);

    SearchThread(const SearchThread&) = delete;
    SearchThread& operator=(const SearchThread&) = delete;

    bool isMainThread() const
    {
        return id == 0;
    }

    void reset();
    void startSearching();
    void initRootMoves();
    void sortRootMoves();
    RootMove& findRootMove(Move move);
    void wait();
    void join();

    u32 id;
    std::thread thread;

    std::mutex mutex;
    std::condition_variable cv;
    WakeFlag wakeFlag;

    Board board;

    std::atomic_uint64_t nodes = 0;

    SearchLimits limits;

    i32 rootDepth = 0;
    i32 rootPly = 0;
    i32 selDepth = 0;
    i32 nmpMinPly = 0;
    std::vector<RootMove> rootMoves;
    std::array<SearchStack, MAX_PLY + 1> stack;
    History history;
    PawnTable pawnTable;
    eval::EvalState evalState;
};

class Search
{
public:
    Search(size_t hash = 64);
    ~Search();

    void newGame();

    void run(const SearchLimits& limits, const Board& board);
    void stop();
    void setThreads(i32 count);
    bool searching() const;
    BenchData benchSearch(i32 depth, const Board& board);
    std::pair<i32, Move> datagenSearch(const SearchLimits& limits, const Board& board);

    void setTTSize(i32 mb)
    {
        m_TT.resize(mb, m_Threads.size());
    }

private:
    void joinThreads();
    void threadLoop(SearchThread& thread);

    void reportUCIInfo(const SearchThread& thread, i32 multiPVIdx, i32 depth) const;

    std::pair<i32, Move> iterDeep(SearchThread& thread, bool report);
    i32 aspWindows(SearchThread& thread, i32 depth, i32 prevScore, bool report);

    i32 search(SearchThread& thread, i32 depth, SearchStack* stack, i32 alpha, i32 beta,
        bool pvNode, bool cutnode);
    i32 qsearch(SearchThread& thread, SearchStack* stack, i32 alpha, i32 beta, bool pvNode);

    void makeMove(SearchThread& thread, SearchStack* stack, Move move, i32 histScore);
    void unmakeMove(SearchThread& thread, SearchStack* stack);
    void makeNullMove(SearchThread& thread, SearchStack* stack);
    void unmakeNullMove(SearchThread& thread, SearchStack* stack);

    std::atomic_bool m_ShouldStop;
    TT m_TT;
    TimeManager m_TimeMan;
    std::deque<BoardState> m_States;

    std::vector<std::unique_ptr<SearchThread>> m_Threads;
};

}

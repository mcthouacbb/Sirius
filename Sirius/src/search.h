#pragma once

#include "board.h"
#include "defs.h"
#include "tt.h"
#include "time_man.h"
#include "history.h"
#include "eval/pawn_table.h"
#include "eval/eval_state.h"

#include <array>
#include <deque>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <vector>

struct SearchStack
{
    std::array<Move, MAX_PLY + 1> pv;
    int pvLength;

    Move playedMove;
    Piece movedPiece;
    Move excludedMove;

    std::array<Move, 2> killers;

    ContCorrEntry* contCorrEntry;
    CHEntry* contHistEntry;
    int histScore;

    int staticEval;
    int eval;

    uint32_t failHighCount;
};

struct SearchInfo
{
    int depth;
    int selDepth;
    int hashfull;
    uint64_t nodes;
    Duration time;
    const Move* pvBegin, * pvEnd;
    int score;
};

struct BenchData
{
    uint64_t nodes;
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

struct SearchThread
{
    SearchThread(uint32_t id, std::thread&& thread);

    SearchThread(const SearchThread&) = delete;
    SearchThread& operator=(const SearchThread&) = delete;

    bool isMainThread() const
    {
        return id == 0;
    }

    void reset();
    void startSearching();
    void wait();
    void join();

    uint32_t id;
    std::thread thread;

    std::mutex mutex;
    std::condition_variable cv;
    WakeFlag wakeFlag;


    Board board;

    std::atomic_uint64_t nodes = 0;

    SearchLimits limits;

    int rootDepth = 0;
    int rootPly = 0;
    int selDepth = 0;
    int nmpMinPly = 0;
    std::array<SearchStack, MAX_PLY + 1> stack;
    History history;
    PawnTable pawnTable;
    eval::EvalState evalState;
};

class Search
{
public:
    Search(Board& board);
    ~Search();

    void newGame();

    void run(const SearchLimits& limits, const Board& board);
    void stop();
    void setThreads(int count);
    bool searching() const;
    BenchData benchSearch(int depth, const Board& board);

    void setTTSize(int mb)
    {
        m_TT.resize(mb);
    }
private:
    void joinThreads();
    void threadLoop(SearchThread& thread);

    int iterDeep(SearchThread& thread, bool report, bool normalSearch);
    int aspWindows(SearchThread& thread, int depth, Move& bestMove, int prevScore);

    int search(SearchThread& thread, int depth, SearchStack* stack, int alpha, int beta, bool pvNode, bool cutnode);
    int qsearch(SearchThread& thread, SearchStack* stack, int alpha, int beta, bool pvNode);

    void makeMove(SearchThread& thread, SearchStack* stack, Move move, int histScore);
    void unmakeMove(SearchThread& thread, SearchStack* stack);
    void makeNullMove(SearchThread& thread, SearchStack* stack);
    void unmakeNullMove(SearchThread& thread, SearchStack* stack);

    Board& m_Board;
    std::atomic_bool m_ShouldStop;
    TT m_TT;
    TimeManager m_TimeMan;
    std::deque<BoardState> m_States;

    std::vector<std::unique_ptr<SearchThread>> m_Threads;
};


}

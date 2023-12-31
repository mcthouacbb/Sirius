#pragma once

#include "board.h"
#include "defs.h"
#include "tt.h"
#include "time_man.h"
#include "history.h"

#include <array>
#include <deque>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <vector>

struct SearchPly
{
    std::array<Move, MAX_PLY + 1> pv;
    int pvLength;

    Move bestMove;
    std::array<Move, 2> killers;

    CHEntry* contHistEntry;
    Move excludedMove;

    int staticEval;
    int eval;

    uint32_t failHighCount;
};

struct SearchInfo
{
    int depth;
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

    uint64_t nodes = 0;

    SearchLimits limits;

    uint32_t checkCounter = 0;
    int rootPly = 0;
    std::array<SearchPly, MAX_PLY + 1> stack;
    History history;
};

class Search
{
public:
    Search(Board& board);
    ~Search();

    void newGame();

    void run(const SearchLimits& limits, const std::deque<BoardState>& states);
    void stop();
    void setThreads(int count);
    bool searching() const;
    BenchData benchSearch(int depth, const Board& board, BoardState& state);

    void setTTSize(int mb)
    {
        m_TT.resize(mb);
    }
private:
    void joinThreads();
    void threadLoop(SearchThread& thread);

    int iterDeep(SearchThread& thread, bool report, bool normalSearch);
    int aspWindows(SearchThread& thread, int depth, Move& bestMove, int prevScore);

    void storeKiller(SearchPly* ply, Move killer);
    int search(SearchThread& thread, int depth, SearchPly* searchPly, int alpha, int beta, bool isPV);
    int qsearch(SearchThread& thread, SearchPly* searchPly, int alpha, int beta);

    Board& m_Board;
    std::atomic<bool> m_ShouldStop;
    TT m_TT;
    TimeManager m_TimeMan;
    std::deque<BoardState> m_States;

    std::vector<std::unique_ptr<SearchThread>> m_Threads;
};


}

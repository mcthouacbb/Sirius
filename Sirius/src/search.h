#pragma once

#include "board.h"
#include "defs.h"
#include "tt.h"
#include "time_man.h"

#include <array>
#include <deque>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <vector>

struct SearchPly
{
    Move* pv;
    int pvLength;
    Move bestMove;
    Move killers[2];
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

    SearchThread(SearchThread&&) noexcept = default;
    SearchThread& operator=(SearchThread&&) noexcept = default;

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
    Move pv[MAX_PLY + 1];
    int history[2][4096];
    SearchPly plies[MAX_PLY + 1];
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
    int aspWindows(SearchThread& thread, int depth, int prevScore);

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

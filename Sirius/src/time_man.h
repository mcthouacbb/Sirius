#pragma once

#include <chrono>
#include <array>
#include "defs.h"

using TimePoint = std::chrono::steady_clock::time_point;
using Duration = std::chrono::milliseconds;

struct SearchLimits
{
    int maxDepth;
    Duration maxTime;
    uint64_t maxNodes;

    struct
    {
        std::array<Duration, 2> timeLeft;
        std::array<Duration, 2> increments;
        bool enabled;
    } clock;

    Duration overhead;
};

class TimeManager
{
public:
    TimeManager() = default;

    void setLimits(const SearchLimits& searchLimits, Color us);
    Duration elapsed() const;

    void startSearch();
    void updateNodes(Move move, uint64_t nodes);
    bool stopHard(const SearchLimits& searchLimits, uint64_t nodes);
    bool stopSoft(Move bestMove, uint64_t totalNodes, const SearchLimits& searchLimits);
private:
    static constexpr uint32_t TIME_CHECK_INTERVAL = 2048;

    TimePoint m_StartTime;
    Duration m_HardBound;
    Duration m_SoftBound;

    uint32_t checkCounter;

    std::array<uint64_t, 4096> m_NodeCounts;
    Move m_PrevBestMove;
    uint32_t m_Stability;
};

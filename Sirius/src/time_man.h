#pragma once

#include "defs.h"
#include <array>
#include <chrono>

using TimePoint = std::chrono::steady_clock::time_point;
using Duration = std::chrono::milliseconds;

struct SearchLimits
{
    i32 maxDepth;
    Duration maxTime;
    u64 maxNodes;
    u64 softNodes;

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
    bool stopHard(const SearchLimits& searchLimits, u64 nodes);
    bool stopSoft(Move bestMove, u64 bmNodes, u64 totalNodes, const SearchLimits& searchLimits);

private:
    static constexpr u32 TIME_CHECK_INTERVAL = 2048;

    TimePoint m_StartTime;
    Duration m_HardBound;
    Duration m_SoftBound;

    u32 checkCounter;

    Move m_PrevBestMove;
    u32 m_Stability;
};

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

    struct
    {
        std::array<Duration, 2> timeLeft;
        std::array<Duration, 2> increments;
        bool enabled;
    } clock;
};

class TimeManager
{
public:
    TimeManager() = default;

    void setLimits(const SearchLimits& searchLimits, Color us);
    Duration elapsed() const;

    void startSearch();
    void updateNodes(Move move, uint64_t nodes);
    bool stopHard(const SearchLimits& searchLimits) const;
    bool stopSoft(Move bestMove, uint64_t totalNodes, const SearchLimits& searchLimits) const;
private:
    TimePoint m_StartTime;
    Duration m_HardBound;
    Duration m_SoftBound;

    std::array<uint64_t, 4096> m_NodeCounts;
};

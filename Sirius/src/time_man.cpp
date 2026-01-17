#include "time_man.h"
#include "search.h"
#include "search_params.h"
#include <cmath>
#include <iostream>

void TimeManager::setLimits(const SearchLimits& limits, Color us)
{
    if (limits.clock.enabled)
    {
        Duration time =
            std::max(Duration(1), limits.clock.timeLeft[static_cast<i32>(us)] - limits.overhead);
        Duration inc = limits.clock.increments[static_cast<i32>(us)];

        // auto baseTime = (time / search::baseTimeScale + inc * search::incrementScale / 100.0);
        // formulas from stormphrax
        // m_SoftBound = std::chrono::duration_cast<Duration>(search::softTimeScale / 100.0 * baseTime);
        // m_HardBound = std::chrono::duration_cast<Duration>(time * (search::hardTimeScale / 100.0));
        m_SoftBound = time / 20 + inc / 2;
        m_HardBound = time / 6;
    }
}

Duration TimeManager::elapsed() const
{
    return std::chrono::duration_cast<Duration>(std::chrono::steady_clock::now() - m_StartTime);
}

void TimeManager::startSearch()
{
    checkCounter = TIME_CHECK_INTERVAL;
    m_StartTime = std::chrono::steady_clock::now();
}

bool TimeManager::stopHard(const SearchLimits& searchLimits, u64 nodes)
{
    if (searchLimits.maxNodes > 0 && nodes > searchLimits.maxNodes)
        return true;
    if (--checkCounter == 0)
    {
        checkCounter = TIME_CHECK_INTERVAL;
        if (searchLimits.maxTime > Duration(0) && elapsed() > searchLimits.maxTime)
            return true;
        if (searchLimits.clock.enabled && elapsed() > m_HardBound)
            return true;
    }
    return false;
}

bool TimeManager::stopSoft(Move bestMove, u64 totalNodes, const SearchLimits& searchLimits)
{
    if (searchLimits.softNodes > 0 && totalNodes > searchLimits.softNodes)
        return true;

    if (searchLimits.clock.enabled && elapsed() > m_SoftBound)
        return true;

    return false;
}

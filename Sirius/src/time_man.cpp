#include "time_man.h"
#include "search.h"
#include <iostream>

void TimeManager::setLimits(const SearchLimits& limits, Color us)
{
    if (limits.clock.enabled)
    {
        Duration time = limits.clock.timeLeft[static_cast<int>(us)];
        Duration inc = limits.clock.increments[static_cast<int>(us)];

        // formulas from stormphrax
        m_SoftBound = std::chrono::duration_cast<Duration>(0.6 * (time / 20 + inc * 3 / 4));
        m_HardBound = time / 2;
    }
}

Duration TimeManager::elapsed() const
{
    return std::chrono::duration_cast<Duration>(std::chrono::steady_clock::now() - m_StartTime);
}

void TimeManager::startSearch()
{
    m_StartTime = std::chrono::steady_clock::now();
}

bool TimeManager::stopHard(const SearchLimits& searchLimits) const
{
    if (searchLimits.maxTime > Duration(0) && elapsed() > searchLimits.maxTime)
        return true;
    if (searchLimits.clock.enabled && elapsed() > m_HardBound)
        return true;
    return false;
}

bool TimeManager::stopSoft(const SearchLimits& searchLimits) const
{
    if (searchLimits.clock.enabled && elapsed() > m_SoftBound)
        return true;
    return false;
}

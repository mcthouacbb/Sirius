#include "time_man.h"
#include "search.h"
#include <iostream>

void TimeManager::setLimits(const SearchLimits& limits, Color us)
{
    if (limits.clock.enabled)
    {
        Duration time = limits.clock.timeLeft[static_cast<int>(us)];
        Duration inc = limits.clock.increments[static_cast<int>(us)];
        m_AllocatedTime = time / 40 + inc / 2;
        if (m_AllocatedTime >= time)
            m_AllocatedTime = time - Duration(500);
        if (m_AllocatedTime < Duration(0))
            m_AllocatedTime = time / 4;
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

bool TimeManager::shouldStop(const SearchLimits& limits) const
{
    if (limits.maxTime > Duration(0) && elapsed() > limits.maxTime)
        return true;
    if (limits.clock.enabled && elapsed() > m_AllocatedTime)
        return true;
    return false;
}

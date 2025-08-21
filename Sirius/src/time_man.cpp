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
            std::max(Duration(1), limits.clock.timeLeft[static_cast<int>(us)] - limits.overhead);
        Duration inc = limits.clock.increments[static_cast<int>(us)];

        auto baseTime = (time / search::baseTimeScale + inc * search::incrementScale / 100.0);
        // formulas from stormphrax
        m_SoftBound = std::chrono::duration_cast<Duration>(search::softTimeScale / 100.0 * baseTime);
        m_HardBound = std::chrono::duration_cast<Duration>(time * (search::hardTimeScale / 100.0));
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
    m_Stability = 0;
    m_PrevBestMove = Move::nullmove();
}

bool TimeManager::stopHard(const SearchLimits& searchLimits, uint64_t nodes)
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

bool TimeManager::stopSoft(
    Move bestMove, uint64_t bmNodes, uint64_t totalNodes, const SearchLimits& searchLimits)
{
    if (searchLimits.softNodes > 0 && totalNodes > searchLimits.softNodes)
        return true;

    if (bestMove == m_PrevBestMove)
        m_Stability++;
    else
        m_Stability = 0;
    m_PrevBestMove = bestMove;

    double nodeFrac = static_cast<double>(bmNodes) / static_cast<double>(totalNodes);
    double nodeScale = [&]()
    {
        double base = static_cast<double>(search::nodeTMBase) / 100.0;
        double scale = static_cast<double>(search::nodeTMScale) / 100.0;
        return (base - nodeFrac) * scale;
    }();

    double bmStabilityScale = [&]()
    {
        double base = static_cast<double>(search::bmStabilityBase) / 100.0;
        double scale = static_cast<double>(search::bmStabilityScale) / 100.0;
        double offset = static_cast<double>(search::bmStabilityOffset) / 100.0;
        double power = static_cast<double>(search::bmStabilityPower) / 100.0;
        double min = static_cast<double>(search::bmStabilityMin) / 100;

        double result = base + scale * std::pow(m_Stability + offset, power);
        return std::max(result, min);
    }();

    double scale = nodeScale * bmStabilityScale;
    if (searchLimits.clock.enabled && elapsed() > m_SoftBound * scale)
        return true;
    return false;
}

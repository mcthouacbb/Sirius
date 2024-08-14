#include "time_man.h"
#include "search.h"
#include "search_params.h"
#include <iostream>

constexpr std::array<double, 7> stabilityValues = {
    2.2, 1.6, 1.4, 1.1, 1, 0.95, 0.9
};

void TimeManager::setLimits(const SearchLimits& limits, Color us)
{
    if (limits.clock.enabled)
    {
        Duration time = std::max(Duration(1), limits.clock.timeLeft[static_cast<int>(us)] - limits.overhead);
        Duration inc = limits.clock.increments[static_cast<int>(us)];

        // formulas from stormphrax
        m_SoftBound = std::chrono::duration_cast<Duration>(search::softTimeScale / 100.0 * (time / search::baseTimeScale + inc * search::incrementScale / 100.0));
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
    std::fill(m_NodeCounts.begin(), m_NodeCounts.end(), 0);
    m_StartTime = std::chrono::steady_clock::now();
    m_Stability = 0;
    m_PrevBestMove = Move();
}

void TimeManager::updateNodes(Move move, uint64_t nodes)
{
    m_NodeCounts[move.fromTo()] += nodes;
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

bool TimeManager::stopSoft(Move bestMove, uint64_t totalNodes, const SearchLimits& searchLimits)
{
    if (bestMove == m_PrevBestMove)
        m_Stability++;
    else
        m_Stability = 0;
    m_PrevBestMove = bestMove;

    double bmNodes = static_cast<double>(m_NodeCounts[bestMove.fromTo()]) / static_cast<double>(totalNodes);
    double scale = ((search::nodeTMBase / 100.0) - bmNodes) * (search::nodeTMScale / 100.0);

    scale *= stabilityValues[std::min(m_Stability, 6u)];
    if (searchLimits.clock.enabled && elapsed() > m_SoftBound * scale)
        return true;
    return false;
}

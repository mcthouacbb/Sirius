#include "time_man.h"
#include "search.h"
#include <iostream>

void TimeManager::setLimits(const SearchLimits& limits, Color us)
{
	m_Limits = &limits;

	switch (limits.policy)
	{
		case SearchPolicy::FIXED_TIME:
			m_AllocatedTime = limits.time;
			std::cout << m_AllocatedTime.count() << std::endl;
			break;
		case SearchPolicy::DYN_CLOCK:
			m_AllocatedTime = limits.clock.timeLeft[static_cast<int>(us)] / 40 + limits.clock.increments[static_cast<int>(us)] / 2;

			if (m_AllocatedTime >= limits.clock.timeLeft[static_cast<int>(us)])
			{
				m_AllocatedTime = limits.clock.timeLeft[static_cast<int>(us)] - Duration(500);
			}

			if (m_AllocatedTime < Duration(0))
				m_AllocatedTime = limits.clock.timeLeft[static_cast<int>(us)] / 4;
			break;
	}
}

Duration TimeManager::elapsed()
{
	return std::chrono::duration_cast<Duration>(std::chrono::steady_clock::now() - m_StartTime);
}

void TimeManager::startSearch()
{
	m_StartTime = std::chrono::steady_clock::now();
}

bool TimeManager::shouldStop(const SearchInfo& searchInfo)
{
	switch (m_Limits->policy)
	{
		case SearchPolicy::INFINITE:
			return false;
		case SearchPolicy::FIXED_TIME:
		case SearchPolicy::DYN_CLOCK:
			return elapsed() > m_AllocatedTime;
	}
}
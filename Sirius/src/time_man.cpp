#include "time_man.h"
#include "search.h"
#include <iostream>

void TimeManager::setLimits(const SearchLimits& limits)
{
	m_Limits = &limits;

	switch (limits.policy)
	{
		case SearchPolicy::FIXED_TIME:
			m_AllocatedTime = limits.time;
			std::cout << m_AllocatedTime.count() << std::endl;
			break;
		case SearchPolicy::DYN_CLOCK:
			m_AllocatedTime = limits.clock.timeLeft[0] / 40 + limits.clock.increments[0] / 2;
		
			if (m_AllocatedTime >= limits.clock.timeLeft[0])
			{
				m_AllocatedTime = limits.clock.timeLeft[0] - Duration(500);
			}
		
			if (m_AllocatedTime < Duration(0))
				m_AllocatedTime = Duration(100);
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
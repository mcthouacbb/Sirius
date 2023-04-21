#include "time_man.h"
#include <iostream>

void TimeManager::setTimeLeft(Duration time, Duration increment)
{
	m_Clock = time;
	m_Increment = increment;

	m_AllocatedTime = m_Clock / 40 + m_Increment / 2;

	if (m_AllocatedTime >= m_Clock)
	{
		m_AllocatedTime = m_Clock - Duration(500);
	}

	if (m_AllocatedTime < Duration(0))
		m_AllocatedTime = Duration(100);
}

Duration TimeManager::elapsed()
{
	return std::chrono::duration_cast<Duration>(std::chrono::steady_clock::now() - m_StartTime);
}

void TimeManager::startSearch()
{
	m_StartTime = std::chrono::steady_clock::now();
}

bool TimeManager::shouldStop()
{
	// std::cout << (m_StartTime - std::chrono::steady_clock::now()).count() << std::endl;
	// std::cout << m_AllocatedTime.count() << std::endl;
	return elapsed() > m_AllocatedTime;
}
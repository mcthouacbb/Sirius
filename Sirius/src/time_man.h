#pragma once

#include <chrono>

using TimePoint = std::chrono::steady_clock::time_point;
using Duration = std::chrono::milliseconds;

class TimeManager
{
public:
	TimeManager() = default;

	void setTimeLeft(Duration time, Duration increment);
	Duration elapsed();

	void startSearch();
	bool shouldStop();
private:
	TimePoint m_StartTime;
	Duration m_AllocatedTime;
	Duration m_Clock;
	Duration m_Increment;
};
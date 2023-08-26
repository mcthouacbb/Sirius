#pragma once

#include <chrono>
#include "defs.h"

using TimePoint = std::chrono::steady_clock::time_point;
using Duration = std::chrono::milliseconds;


enum class SearchPolicy
{
	INFINITE,
	FIXED_TIME,
	DYN_CLOCK
};

struct SearchLimits
{
	SearchPolicy policy;
	int maxDepth;
	union
	{
		Duration time;
		struct
		{
			Duration timeLeft[2];
			Duration increments[2];
		} clock;
	};
};

class TimeManager
{
public:
	TimeManager() = default;

	void setLimits(const SearchLimits& searchLimits, Color us);
	Duration elapsed();

	void startSearch();
	bool shouldStop(const SearchLimits& searchLimits);
private:
	TimePoint m_StartTime;
	Duration m_AllocatedTime;
};
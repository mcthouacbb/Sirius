#pragma once

#include <chrono>
#include "defs.h"

using TimePoint = std::chrono::steady_clock::time_point;
using Duration = std::chrono::milliseconds;

struct SearchLimits
{
	int maxDepth;
	Duration maxTime;

	struct
	{
		Duration timeLeft[2];
		Duration increments[2];
		bool enabled;
	} clock;
};

class TimeManager
{
public:
	TimeManager() = default;

	void setLimits(const SearchLimits& searchLimits, Color us);
	Duration elapsed() const;

	void startSearch();
	bool shouldStop(const SearchLimits& searchLimits) const;
private:
	TimePoint m_StartTime;
	Duration m_AllocatedTime;
};

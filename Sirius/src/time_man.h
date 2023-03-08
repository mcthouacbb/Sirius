#pragma once

#include <chrono>
#include "defs.h"

using TimePoint = std::chrono::steady_clock::time_point;
using Duration = std::chrono::milliseconds;

struct SearchLimits;
struct SearchInfo;

class TimeManager
{
public:
	TimeManager() = default;

	void setLimits(const SearchLimits& searchLimits, Color us);
	Duration elapsed();

	void startSearch();
	bool shouldStop(const SearchInfo& searchInfo);
private:
	TimePoint m_StartTime;
	Duration m_AllocatedTime;
	const SearchLimits* m_Limits;
};
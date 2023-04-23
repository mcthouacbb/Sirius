#pragma once

#include <chrono>

using TimePoint = std::chrono::steady_clock::time_point;
using Duration = std::chrono::milliseconds;

struct SearchLimits;
struct SearchInfo;

class TimeManager
{
public:
	TimeManager() = default;

	void setLimits(const SearchLimits& searchLimits);
	Duration elapsed();

	void startSearch();
	bool shouldStop(const SearchInfo& searchInfo);
private:
	TimePoint m_StartTime;
	Duration m_AllocatedTime;
	const SearchLimits* m_Limits;
};
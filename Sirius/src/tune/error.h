#pragma once

#include "eval_params.h"
#include "pos.h"
#include <span>
#include <mutex>
#include <thread>
#include <condition_variable>

namespace tune
{

inline double sigmoid(double s, double k)
{
	return 1.0 / (1 + std::pow(10, -k * s / static_cast<double>(NUM_PARAMS)));
}

class ErrorThread {
public:
	ErrorThread();
	~ErrorThread() = default;

	struct Result
	{
	public:
		Result(ErrorThread& thread);
		Result(const Result&) = default;
		Result& operator=(const Result&) = default;
		double waitForResult();
	private:
		ErrorThread* m_Thread;
	};

	Result process(const std::span<const Pos>& positions, const EvalParams& params, double kValue);
	void quit();
private:
	void wait();

	friend Result;

	std::mutex m_Mutex;
	std::condition_variable m_Condition;
	bool m_IsReady;
	bool m_Quit;
	bool m_Finished;
	double m_Result;

	std::span<const Pos> m_Positions;
	EvalParams m_Params;
	double m_kValue;
	std::jthread m_Thread;
};

double error(const std::span<const Pos>& positions, const EvalParams& params, double kValue);
double error(const std::vector<Pos>& positions, const EvalParams& params, double kValue, std::vector<ErrorThread>& threads);

}
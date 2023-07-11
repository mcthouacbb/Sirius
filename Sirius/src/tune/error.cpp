#include "error.h"

#include <chrono>

namespace tune
{

ErrorThread::Result::Result(ErrorThread& thread)
	: m_Thread(&thread)
{

}

double ErrorThread::Result::waitForResult()
{
	std::unique_lock lk(m_Thread->m_Mutex);
	m_Thread->m_Condition.wait(lk, [this]()
	{
		return m_Thread->m_Finished;
	});
	return m_Thread->m_Result;
}

ErrorThread::ErrorThread()
	: m_IsReady(false), m_Quit(false), m_Thread(&ErrorThread::wait, this)
{

}

ErrorThread::Result ErrorThread::process(const std::span<const Pos>& positions, const EvalParams& params, double kValue)
{
	{
		std::lock_guard lg(m_Mutex);
		m_Positions = positions;
		m_Params = params;
		m_kValue = kValue;
		m_IsReady = true;
		m_Finished = false;
	}

	m_Condition.notify_one();

	return Result(*this);
}

void ErrorThread::quit()
{
	{
		std::lock_guard lg(m_Mutex);
		m_Quit = true;
	}
	m_Condition.notify_one();
	m_Thread.join();
}

void ErrorThread::wait()
{
	while (true)
	{
		//std::cout << "Bruh moment" << std::endl;
		std::unique_lock lk(m_Mutex);
		m_Condition.wait(lk, [this](){ return m_IsReady || m_Quit; });
		if (m_Quit)
			return;

		m_Result = error(m_Positions, m_Params, m_kValue);

		m_Finished = true;
		m_IsReady = false;

		lk.unlock();
		m_Condition.notify_one();
	}
}

double error(const std::span<const Pos>& positions, const EvalParams& params, double kValue)
{
	if (isDegenerate(params))
		return 1000000.0;
	double result = 0.0;
	Board board;
	EvalCache cache = genCache(params);

	int maxNodes = 0;
	int totalNodes = 0;
	std::string_view maxEpd;
	for (const auto& pos : positions)
	{
		board.setToEpd(std::string_view(pos.epd, pos.epdLen));
		int nodes = 0;
		int eval = evaluate(board, params, cache);
		totalNodes += nodes;
		if (nodes > maxNodes)
		{
			maxNodes = nodes;
			maxEpd = std::string_view(pos.epd, pos.epdLen);
		}
		double term = pos.result - sigmoid(eval, kValue);
		// std::cout << term << std::endl;

		result += term * term;
	}
	//std::cout << maxNodes << std::endl;
	//std::cout << maxEpd << std::endl;
	//std::cout << totalNodes << std::endl;
	//std::cout << result << std::endl;
	//std::cout << positions.size() << std::endl;
	//result /= static_cast<double>(positions.size());
	return result;
}

double error(const std::vector<Pos>& positions, const EvalParams& params, double kValue, std::vector<ErrorThread>& threads)
{
	std::vector<ErrorThread::Result> results;
	results.reserve(threads.size());
	const int BLOCK_COUNT = static_cast<int>(threads.size() + 1);
	for (uint32_t i = 0; i < threads.size(); i++)
	{
		std::span<const Pos> threadPositions(positions.data() + positions.size() * (i + 1) / BLOCK_COUNT, positions.data() + positions.size() * (i + 2) / BLOCK_COUNT);
		results.push_back(threads[i].process(threadPositions, params, kValue));
	}

	std::span<const Pos> mainThreadPositions(positions.data(), positions.data() + positions.size() / BLOCK_COUNT);
	double result = error(mainThreadPositions, params, kValue);
	for (auto& errorResult : results)
	{
		result += errorResult.waitForResult();
	}

	return result / positions.size();
}


}
#pragma once

#include <queue>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>

class InputQueue
{
public:
	InputQueue(bool (*shouldQuit)(const std::string& str));
	~InputQueue();

	bool hasInput()
	{
		return !m_QueuedInputs.empty();
	}
	std::string pop()
	{
		std::string elem = m_QueuedInputs.front();
		m_QueuedInputs.pop();
		return elem;
	}
	std::mutex& mutex()
	{
		return m_QueueLock;
	}
	std::condition_variable& cond()
	{
		return m_Cond;
	}
private:
	void pollInput();

	std::mutex m_QueueLock;
	std::condition_variable m_Cond;
	bool (*m_ShouldQuit)(const std::string& str);
	std::queue<std::string> m_QueuedInputs;
	std::thread m_InputThread;
};
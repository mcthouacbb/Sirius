#pragma once

#include <queue>
#include <string>
#include <thread>
#include <mutex>

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
	void lock()
	{
		m_QueueLock.lock();
	}
	void unlock()
	{
		m_QueueLock.unlock();
	}
private:
	void pollInput();

	std::thread m_InputThread;
	std::mutex m_QueueLock;
	bool (*m_ShouldQuit)(const std::string& str);
	std::queue<std::string> m_QueuedInputs;
};
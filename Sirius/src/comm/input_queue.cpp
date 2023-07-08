#include "input_queue.h"
#include <iostream>

InputQueue::InputQueue(bool (*shouldQuit)(const std::string& str))
	: m_ShouldQuit(shouldQuit), m_InputThread(&InputQueue::pollInput, this)
{

}

InputQueue::~InputQueue()
{
	m_InputThread.join();
}

void InputQueue::pollInput()
{
	std::string input;
	do
	{
		std::getline(std::cin, input);
		mutex().lock();
		m_QueuedInputs.push(input);
		cond().notify_one();
		mutex().unlock();
	}
	while (!m_ShouldQuit(input));
}
#include "input_queue.h"
#include <iostream>

InputQueue::InputQueue(bool (*shouldQuit)(const std::string& str))
	: m_InputThread(&InputQueue::pollInput, this), m_ShouldQuit(shouldQuit)
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
		lock();
		m_QueuedInputs.push(input);
		unlock();
	}
	while (!m_ShouldQuit(input));
}
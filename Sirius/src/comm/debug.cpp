#include <string>
#include <fstream>

#include "debug.h"
#include "fen.h"
#include "move.h"
#include "../eval/eval.h"
#include "../misc.h"

namespace comm
{

bool Debug::shouldQuit(const std::string& str)
{
	return str == "quit";
}

Debug::Debug()
	: m_InputQueue(shouldQuit)
{

}

void Debug::run()
{
	for (;;)
	{
		std::unique_lock<std::mutex> lock(m_InputQueue.mutex());
		m_InputQueue.cond().wait(
			lock,
			[this]{return m_InputQueue.hasInput();}
		);

		while (m_InputQueue.hasInput())
		{
			std::string input = m_InputQueue.pop();
			lock.unlock();
			execCommand(input);
			if (m_State == CommState::QUITTING)
				return;
			lock.lock();
		}

		lock.unlock();
	}
}

void Debug::reportSearchInfo(const SearchInfo& info) const
{
	
}

bool Debug::checkInput()
{
	
}

void Debug::execCommand(const std::string& command)
{
	std::istringstream stream(command);

	std::string tok;
	stream >> tok;
	Command comm = getCommand(tok);

	switch (comm)
	{
		case Command::INVALID:
			std::cout << "Invalid command: " << command << std::endl;
			break;
		case Command::EPD_EVALS:
			genEpdEvals(stream);
			break;
		case Command::QUIT:
			m_State = CommState::QUITTING;
			break;
	}
}

Debug::Command Debug::getCommand(const std::string& command) const
{
	if (command == "quit")
		return Command::QUIT;
	else if (command == "genepdevals")
		return Command::EPD_EVALS;

	return Command::INVALID;
}

void Debug::genEpdEvals(std::istringstream& stream) const
{
	std::string filename;
	
	std::string tok;
	stream >> tok;
	if (tok[0] == '"')
	{
		filename = tok;
		do
		{
			if (!stream)
			{
				std::cout << "Invalid file name" << std::endl;
				return;
			}
			stream >> tok;
			filename += tok;
		}
		while (tok[tok.size() - 1] != '"');

		filename = filename.substr(1, filename.size() - 2);
	}
	else
	{
		filename = tok;
	}

	std::cout << "FILENAME: " << filename << std::endl;

	std::ifstream inFile(filename);

	std::ofstream outFile("evals.txt");

	Board board;

	while (std::getline(inFile, tok))
	{
		// tok = tok.substr(0, tok.size() - 1);
		// std::cout << "TOK: "
		// std::cout << "LSCHAR " << tok[tok.size() - 1] << std::endl;
		switch (tok[tok.size() - 1])
		{
			case '2':
				// std::cout << "Removing last 7 characters" << std::endl;
				tok = tok.substr(0, tok.size() - 7);
				break;
			case '1':
			case '0':
				// std::cout << "Removing last 3 characters" << std::endl;
				tok = tok.substr(0, tok.size() - 3);
				break;
		}
		tok += "0 1";

		if (!isValidFen(tok.c_str()))
			throw std::runtime_error(tok + "idek bro");

		board.setToFen(tok);
		int eval = eval::rawEval(board) * (board.sideToMove() == Color::WHITE ? 1 : -1);
		outFile << eval << '\n';
	}
}


}
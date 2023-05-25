#include <string>
#include <fstream>

#include "debug.h"
#include "fen.h"
#include "move.h"
#include "../eval/eval.h"
#include "../misc.h"
#include "pgn.h"

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
	return false;
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
		case Command::PGN_TO_EPD:
			pgnToEpd(stream);
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
	else if (command == "pgntoepd")
		return Command::PGN_TO_EPD;

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

void Debug::pgnToEpd(std::istringstream& stream) const
{
	std::string pgnFilename;
	stream >> pgnFilename;

	PGNFile pgnFile(pgnFilename.c_str());

	std::string epdFilename;
	stream >> epdFilename;
	std::ofstream epdFile(epdFilename, std::ios::app);
	
	while (pgnFile.hasGame())
	{
		PGNGame game = pgnFile.parseGame();
	
		for (const auto& pair : game.header.tags)
		{
			std::cout << "[" << pair.first << ',' << pair.second << "], ";
		}
		std::cout << std::endl;
		if (game.header.tags["Result"] == "*")
		{
			std::cout << "No Result" << std::endl;
			continue;
		}
		Board board;
		// printBoard(board);
		std::deque<BoardState> states;
		for (const auto& entry : game.entries)
		{
			if (entry.comment == "book")
			{
				std::cout << "Skipping book position" << std::endl;
				continue;
			}
			int sign = 1;
			const char* p = entry.comment.c_str();
			if (p[0] == '+')
				p++;
			if (p[0] == '-')
			{
				sign = -1;
				p++;
			}

			if (p[0] == 'M')
			{
				std::cout << "Skipping mate position" << std::endl;
				continue;
			}
			std::cout << "{" << entry.comment << "}\n";
			
			states.push_back({});
			board.makeMove(entry.move, states.back());
			// printBoard(board);
			epdFile << board.epdStr() << ' ' << game.header.tags["Result"] << '\n';
		}
		std::cout << std::endl;
	}
}


}
#include <string>

#include "../sirius.h"
#include "uci.h"
#include "fen.h"
#include "move.h"
#include "../eval/eval.h"
#include "../misc.h"

namespace comm
{

bool UCI::shouldQuit(const std::string& str)
{
	return str == "quit";
}

UCI::UCI()
	: m_InputQueue(shouldQuit)
{

}

void UCI::run()
{
	uciCommand();

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

void UCI::reportSearchInfo(const SearchInfo& info) const
{
	std::cout << "info depth " << info.depth;
	std::cout << " time " << info.time.count();
	std::cout << " nodes " << info.nodes;
	uint64_t nps = info.nodes * 1000ULL / (info.time.count() < 1 ? 1 : info.time.count());
	std::cout << " nps " << nps;
	std::cout << " score ";
	if (eval::isMateScore(info.score))
	{
		if (info.score > 0)
		{
			std::cout << "mate " << ((-eval::CHECKMATE - info.score) + 1) / 2;
		}
		else
		{
			std::cout << "mate -" << (info.score - eval::CHECKMATE) / 2;
		}
	}
	else
	{
		std::cout << " cp " << info.score;
	}

	std::cout << " pv ";
	for (const Move* move = info.pvBegin; move != info.pvEnd; move++)
	{
		std::cout << comm::convMoveToPCN(*move) << ' ';
	}
	std::cout << std::endl;
}

bool UCI::checkInput()
{
	m_InputQueue.mutex().lock();
	while (m_InputQueue.hasInput())
	{
		execCommand(m_InputQueue.pop());
	}
	m_InputQueue.mutex().unlock();
	return m_State == CommState::ABORTING || m_State == CommState::QUITTING;
}


void UCI::execCommand(const std::string& command)
{
	std::istringstream stream(command);

	std::string tok;
	stream >> tok;
	Command comm = getCommand(tok);

	switch (comm)
	{
		case Command::INVALID:
			break;
		case Command::UCI:
			uciCommand();
			break;
		case Command::IS_READY:
			std::cout << "readyok" << std::endl;
			break;
		case Command::NEW_GAME:
			if (m_State == CommState::IDLE)
				newGameCommand();
			break;
		case Command::POSITION:
			if (m_State == CommState::IDLE)
				positionCommand(stream);
			break;
		case Command::GO:
			if (m_State == CommState::IDLE)
				goCommand(stream);
			break;
		case Command::STOP:
			if (m_State == CommState::SEARCHING)
				m_State = CommState::ABORTING;
			break;
		case Command::QUIT:
			m_State = CommState::QUITTING;
			break;
		// non standard commands
		case Command::DBG_PRINT:
			printBoard(m_Board);
			break;
	}
}

UCI::Command UCI::getCommand(const std::string& command) const
{
	if (command == "uci")
		return Command::UCI;
	else if (command == "isready")
		return Command::IS_READY;
	else if (command == "ucinewgame")
		return Command::NEW_GAME;
	else if (command == "position")
			return Command::POSITION;
	else if (command == "go")
		return Command::GO;
	else if (command == "stop")
		return Command::STOP;
	else if (command == "quit")
		return Command::QUIT;
	else if (command == "d")
		return Command::DBG_PRINT;

	return Command::INVALID;
}


void UCI::uciCommand() const
{
	std::cout << "id name Sirius v" << SIRIUS_VERSION_STRING << std::endl;
	std::cout << "id author AspectOfTheNoob\n";
	std::cout << "uciok" << std::endl;
}

void UCI::newGameCommand()
{
	// lol
}

void UCI::positionCommand(std::istringstream& stream)
{
	std::string tok;
	stream >> tok;

	if (tok == "startpos")
	{
		setToFen(Board::defaultFen);
		if (stream)
		{
			stream >> tok;
			if (tok == "moves")
			{
				while (stream.tellg() != -1)
				{
					stream >> tok;
					MoveStrFind find = comm::findMoveFromPCN(m_LegalMoves, m_LegalMoves + m_MoveCount, tok.c_str());
					Move move = *find.move;
					makeMove(move);
				}
			}
		}
	}
	else if (tok == "fen")
	{
		std::string fen;
		stream >> fen;
		while (stream.tellg() != -1)
		{
			stream >> tok;
			if (tok == "moves")
				break;
			fen += ' ' + tok;
		}

		if (!comm::isValidFen(fen.c_str()))
			return;
		setToFen(fen.c_str());

		if (tok == "moves")
		{
			while (stream.tellg() != -1)
			{
				stream >> tok;
				MoveStrFind find = comm::findMoveFromPCN(m_LegalMoves, m_LegalMoves + m_MoveCount, tok.c_str());
				Move move = *find.move;
				makeMove(move);
			}
		}
	}
	else
		return;
}

void UCI::goCommand(std::istringstream& stream)
{
	std::string tok;
	SearchLimits limits = {};
	limits.policy = SearchPolicy::INFINITE;
	limits.maxDepth = 1000;
	while (stream.tellg() != -1)
	{
		stream >> tok;
		if (tok == "wtime")
		{
			int wtime;
			stream >> wtime;
			limits.clock.timeLeft[static_cast<int>(Color::WHITE)] = Duration(wtime);
			limits.policy = SearchPolicy::DYN_CLOCK;
		}
		else if (tok == "btime")
		{
			int btime;
			stream >> btime;
			limits.clock.timeLeft[static_cast<int>(Color::BLACK)] = Duration(btime);
			limits.policy = SearchPolicy::DYN_CLOCK;
		}
		else if (tok == "winc")
		{
			int winc;
			stream >> winc;
			limits.clock.increments[static_cast<int>(Color::WHITE)] = Duration(winc);
			limits.policy = SearchPolicy::DYN_CLOCK;
		}
		else if (tok == "binc")
		{
			int binc;
			stream >> binc;
			limits.clock.increments[static_cast<int>(Color::BLACK)] = Duration(binc);
			limits.policy = SearchPolicy::DYN_CLOCK;
		}
		else if (tok == "movestogo")
		{
			// todo
		}
		else if (tok == "depth")
		{
			int depth;
			stream >> depth;
			limits.maxDepth = depth;
		}
		else if (tok == "nodes")
		{
			// todo
		}
		else if (tok == "mate")
		{
			// todo
		}
		else if (tok == "movetime")
		{
			int time;
			stream >> time;
			limits.time = Duration(time);
			limits.policy = SearchPolicy::FIXED_TIME;
		}
		else if (tok == "infinite")
		{
			limits.policy = SearchPolicy::INFINITE;
		}
	}

	m_State = CommState::SEARCHING;
	m_Search.iterDeep(limits);
	if (m_State == CommState::QUITTING)
		return;
	m_State = CommState::IDLE;

	std::cout << "bestmove " << comm::convMoveToPCN(m_Search.info().pvBegin[0]) << std::endl;
}


}
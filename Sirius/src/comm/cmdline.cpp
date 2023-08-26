/*#include "../sirius.h"
#include "cmdline.h"
#include "fen.h"
#include "move.h"
#include "../eval/eval.h"
#include "../misc.h"
#include "../movegen.h"

#include <fstream>
#include <sstream>

namespace comm
{

bool CmdLine::shouldQuit(const std::string& str)
{
	return str == "quit";
}

CmdLine::CmdLine()
	: m_InputQueue(shouldQuit)
{
	std::ifstream openings("res/gaviota_trim.pgn");

	std::ostringstream sstr;
	sstr << openings.rdbuf();
	std::string pgnData = sstr.str();

	std::cout << "Sirius v" << SIRIUS_VERSION_STRING << std::endl;

	m_Book.loadFromPGN(pgnData.c_str());
}

void CmdLine::run()
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

void CmdLine::reportSearchInfo(const SearchInfo& info) const
{
	float time = std::chrono::duration_cast<std::chrono::duration<float>>(info.time).count();
	std::cout << "Depth: " << info.depth << '\n';
	std::cout << "\tNodes: " << info.nodes << '\n';
	std::cout << "\tTime Searched: " << time << '\n';
	std::cout << "\tScore: ";
	if (eval::isMateScore(info.score))
	{
		if (info.score < 0)
			std::cout << "Mated in " << info.score + SCORE_MATE << " plies\n";
		else
			std::cout << "Mate in " << SCORE_MATE - info.score << " plies\n";
	}
	else
	{
		std::cout << info.score << '\n';
	}

	std::cout << "PV: ";

	Board board;
	board.setToFen(m_Board.fenStr());

	Move moves[256], *end;

	std::deque<BoardState> states;

	for (const Move* move = info.pvBegin; move != info.pvEnd; move++)
	{
		end = genMoves<MoveGenType::LEGAL>(board, moves);
		std::cout << comm::convMoveToSAN(board, moves, end, *move) << ' ';
		states.push_back({});
		board.makeMove(*move, states.back());
	}

	std::cout << std::endl;
}

void CmdLine::execCommand(const std::string& command)
{
	std::istringstream stream(command);

	std::string commandName;

	stream >> commandName;

	Command comm = getCommand(commandName);

	switch (comm)
	{
		case Command::INVALID:
			std::cout << "Invalid Command: " << command << std::endl;
			break;
		case Command::SET_POSITION:
			if (m_State != CommState::IDLE)
			{
				std::cout << "Error: Cannot set position while thinking" << std::endl;
				break;
			}
			setPositionCommand(stream);
			break;
		case Command::MAKE_MOVE:
			if (m_State != CommState::IDLE)
			{
				std::cout << "Error: Cannot make move while thinking" << std::endl;
				break;
			}
			makeMoveCommand(stream);
			break;
		case Command::UNDO_MOVE:
			if (m_State != CommState::IDLE)
			{
				std::cout << "Error: Cannot undo move while thinking" << std::endl;
				break;
			}
			undoMoveCommand();
			break;
		case Command::PRINT_BOARD:
			if (m_State != CommState::IDLE)
			{
				std::cout << "Error: Cannot print board while thinking" << std::endl;
				break;
			}
			printBoardCommand();
			break;
		case Command::STATIC_EVAL:
			if (m_State != CommState::IDLE)
			{
				std::cout << "Error: Cannot print static eval while thinking" << std::endl;
				break;
			}
			staticEvalCommand();
			break;
		case Command::QUIESCENCE_EVAL:
			if (m_State != CommState::IDLE)
			{
				std::cout << "Error: Cannot print quiescence eval while thinking" << std::endl;
				break;
			}
			quiescenceEvalCommand();
			break;
		case Command::SEARCH:
			if (m_State != CommState::IDLE)
			{
				std::cout << "Error: Cannot start search while already searching\n";
				std::cout << "Abort the search or wait for it to finish to start a new one" << std::endl;
				break;
			}
			searchCommand(stream);
			break;
		case Command::RUN_TESTS:
			if (m_State != CommState::IDLE)
			{
				std::cout << "Cannot run tests while thinking" << std::endl;
				break;
			}
			runTestsCommand();
			break;
		case Command::PERFT:
			if (m_State != CommState::IDLE)
			{
				std::cout << "Cannot run perft while thinking" << std::endl;
				break;
			}
			runPerftCommand(stream);
			break;
		case Command::BOOK:
			if (m_State != CommState::IDLE)
			{
				std::cout << "Cannot probe book while thinking" << std::endl;
				break;
			}
			probeBookCommand();
			break;
		case Command::STOP:
			if (m_State != CommState::SEARCHING)
			{
				std::cout << "No search to stop" << std::endl;
				break;
			}
			m_State = CommState::ABORTING;
			std::cout << "Aborting search" << std::endl;
			break;
		case Command::QUIT:
			m_State = CommState::QUITTING;
			std::cout << "Quitting" << std::endl;
			break;;
	}
}

bool CmdLine::checkInput()
{
	m_InputQueue.mutex().lock();
	while (m_InputQueue.hasInput())
	{
		execCommand(m_InputQueue.pop());
	}
	m_InputQueue.mutex().unlock();
	return m_State == CommState::ABORTING || m_State == CommState::QUITTING;
}

CmdLine::Command CmdLine::getCommand(const std::string& command) const
{
	if (command == "position")
		return Command::SET_POSITION;
	else if (command == "move")
		return Command::MAKE_MOVE;
	else if (command == "undo")
		return Command::UNDO_MOVE;
	else if (command == "print")
		return Command::PRINT_BOARD;
	else if (command == "eval")
		return Command::STATIC_EVAL;
	else if (command == "qeval")
		return Command::QUIESCENCE_EVAL;
	else if (command == "search")
		return Command::SEARCH;
	else if (command == "tests")
		return Command::RUN_TESTS;
	else if (command == "perft")
		return Command::PERFT;
	else if (command == "book")
		return Command::BOOK;
	else if (command == "stop")
		return Command::STOP;
	else if (command == "quit")
		return Command::QUIT;

	return Command::INVALID;
}

void CmdLine::setPositionCommand(std::istringstream& stream)
{
	std::string tok;
	stream >> tok;
	if (tok == "fen")
	{
		std::string str = stream.str();
		stream.ignore();
		const char* fen = str.c_str() + stream.tellg();
		std::cout << "fen: " << fen << std::endl;
		if (!comm::isValidFen(fen))
		{
			std::cout << "Invalid fen string" << std::endl;;
			return;
		}
		setToFen(fen);
	}
	else if (tok == "startpos")
	{
		setToFen(Board::defaultFen);
	}
	else
	{
		std::cout << "Invalid position" << std::endl;
	}
}

void CmdLine::makeMoveCommand(std::istringstream& stream)
{
	std::string move;
	stream >> move;
	MoveStrFind find = comm::findMoveFromSAN(m_Board, m_LegalMoves, m_LegalMoves + m_MoveCount, move.c_str());
	if (find.move == nullptr)
	{
		std::cout << "Invalid move string" << std::endl;
		return;
	}

	if (find.move == m_LegalMoves + m_MoveCount)
	{
		std::cout << "Move not found" << std::endl;
		return;
	}

	if (find.move == m_LegalMoves + m_MoveCount + 1)
	{
		std::cout << "Move is ambiguous" << std::endl;
		return;
	}

	makeMove(*find.move);
}

void CmdLine::undoMoveCommand()
{
	if (m_PrevMoves.empty())
	{
		std::cout << "No moves to undo" << std::endl;
		return;
	}

	unmakeMove();
}

void CmdLine::printBoardCommand()
{
	printBoard(m_Board);
}

void CmdLine::staticEvalCommand()
{
	std::cout << "Eval: " << eval::evaluate(m_Board) << std::endl;
	std::cout << "Phase: " << m_Board.evalState().phase << std::endl;
	std::cout << "Eval midgame:\n";
	std::cout << "\tMaterial:\n";
	std::cout << "\t\tWhite: " << m_Board.evalState().materialMG[0] << '\n';
	std::cout << "\t\tBlack: " << m_Board.evalState().materialMG[1] << '\n';
	std::cout << "\tPiece Square Tables:\n";
	std::cout << "\t\tWhite: " << m_Board.evalState().psqtMG[0] << '\n';
	std::cout << "\t\tBlack: " << m_Board.evalState().psqtMG[1] << '\n';
	std::cout << "Eval endgame:\n";
	std::cout << "\tMaterial:\n";
	std::cout << "\t\tWhite: " << m_Board.evalState().materialEG[0] << '\n';
	std::cout << "\t\tBlack: " << m_Board.evalState().materialEG[1] << '\n';
	std::cout << "\tPiece Square Tables:\n";
	std::cout << "\t\tWhite: " << m_Board.evalState().psqtEG[0] << '\n';
	std::cout << "\t\tBlack: " << m_Board.evalState().psqtEG[1] << '\n';
}

void CmdLine::quiescenceEvalCommand()
{
	int eval = m_Search.qsearch();
	std::cout << "Quiescence eval: " << eval << std::endl;
}

void CmdLine::searchCommand(std::istringstream& stream)
{
	SearchLimits limits;
	limits.maxDepth = 1000;

	std::string tok;
	stream >> tok;
	if (tok == "infinite")
	{
		limits.policy = SearchPolicy::INFINITE;
	}
	else if (tok == "depth")
	{
		uint32_t depth;
		stream >> depth;
		limits.policy = SearchPolicy::INFINITE;
		limits.maxDepth = depth;
	}
	else if (tok == "time")
	{
		uint32_t millis;
		stream >> millis;
		limits.policy = SearchPolicy::FIXED_TIME;
		limits.time = Duration(millis);
	}
	else
	{
		std::cout << "No search policy specified, defaulting to infinite" << std::endl;
		limits.policy = SearchPolicy::INFINITE;
	}


	m_State = CommState::SEARCHING;
	auto t1 = std::chrono::steady_clock::now();
	int eval = m_Search.iterDeep(limits, true);
	auto t2 = std::chrono::steady_clock::now();
	if (m_State == CommState::QUITTING)
		return;
	m_State = CommState::IDLE;

	auto time = std::chrono::duration_cast<std::chrono::duration<float>>(t2 - t1);

	std::cout << "Eval: " << eval << std::endl;
}

void CmdLine::runTestsCommand()
{
	runTests(m_Board, false);
}

void CmdLine::runPerftCommand(std::istringstream& stream)
{
	uint32_t depth;
	stream >> depth;

	auto t1 = std::chrono::steady_clock::now();
	uint64_t result = perft<true>(m_Board, depth);
	auto t2 = std::chrono::steady_clock::now();
	std::cout << "Nodes: " << result << std::endl;
	std::cout << "Time: " << std::chrono::duration_cast<std::chrono::duration<float>>(t2 - t1).count() << std::endl;
}

void CmdLine::probeBookCommand()
{
	const std::vector<BookEntry>* entries = m_Book.lookup(m_Board.zkey());

	if (!entries)
		std::cout << "No moves in book found" << std::endl;
	else
	{
		for (const auto& entry : *entries)
		{
			std::cout << comm::convMoveToSAN(m_Board, m_LegalMoves, m_LegalMoves + m_MoveCount, entry.move) << ' ';
		}
		std::cout << std::endl;
	}
}



}*/
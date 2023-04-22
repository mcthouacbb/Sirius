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

CmdLine::CmdLine()
{
	m_Search.setTime(Duration(180000), Duration(1000));
	std::ifstream openings("res/gaviota_trim.pgn");

	std::ostringstream sstr;
	sstr << openings.rdbuf();
	std::string pgnData = sstr.str();

	m_Book.loadFromPGN(pgnData.c_str());
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
			std::cout << "Mated in " << info.score - eval::CHECKMATE << " plies\n";
		else
			std::cout << "Mate in " << -eval::CHECKMATE - info.score << " plies\n";
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
			setPositionCommand(stream);
			break;
		case Command::MAKE_MOVE:
			makeMoveCommand(stream);
			break;
		case Command::UNDO_MOVE:
			undoMoveCommand();
			break;
		case Command::PRINT_BOARD:
			printBoardCommand();
			break;
		case Command::STATIC_EVAL:
			staticEvalCommand();
			break;
		case Command::QUIESCENCE_EVAL:
			quiescenceEvalCommand();
			break;
		case Command::SEARCH:
			searchCommand(stream);
			break;
		case Command::RUN_TESTS:
			runTestsCommand();
			break;
		case Command::PERFT:
			runPerftCommand(stream);
			break;
		case Command::BOOK:
			probeBookCommand(stream);
			break;
	}
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
	std::cout << m_Board.stringRep() << std::endl;
	std::cout << "GamePly: " << m_Board.gamePly() << std::endl;
	std::cout << "HalfMoveClock: " << m_Board.halfMoveClock() << std::endl;
	std::cout << "CastlingRights: " << m_Board.castlingRights() << std::endl;
	std::cout << "Side to move: " << (m_Board.sideToMove() == Color::WHITE ? "WHITE" : "BLACK") << std::endl;
	if (m_Board.epSquare() != 0)
		std::cout << "Ep square: " << static_cast<char>((m_Board.epSquare() & 7) + 'a') << static_cast<char>((m_Board.epSquare() >> 3) + '1') << std::endl;
	else
		std::cout << "Ep square: N/A" << std::endl;
	std::cout << "Fen: " <<  m_Board.fenStr() << std::endl;

	std::cout << "Zobrist hash: " << m_Board.zkey().value << std::endl;
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
	int eval = m_Search.qsearch(eval::NEG_INF, eval::POS_INF);
	std::cout << "Quiescence eval: " << eval << std::endl;
}

void CmdLine::searchCommand(std::istringstream& stream)
{
	uint32_t depth;
	stream >> depth;

	auto t1 = std::chrono::steady_clock::now();
	int depthSearched;
	int eval = m_Search.iterDeep(depth, depthSearched);
	auto t2 = std::chrono::steady_clock::now();

	auto time = std::chrono::duration_cast<std::chrono::duration<float>>(t2 - t1);

	std::cout << "Time: " << time.count() << std::endl;
	std::cout << "Depth: " << depthSearched << std::endl;
	std::cout << "Eval: " << eval << std::endl;
	std::cout << "PV: ";
	for (const Move* move = m_Search.info().pvBegin; move != m_Search.info().pvEnd; move++)
	{
		std::cout << comm::convMoveToSAN(m_Board, m_LegalMoves, m_LegalMoves + m_MoveCount, *move) << ' ';
		makeMove(*move);
	}

	for (int i = 0; i < m_Search.info().pvEnd - m_Search.info().pvBegin; i++)
		unmakeMove();
	std::cout << std::endl;
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

void CmdLine::probeBookCommand(std::istringstream& stream)
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



}
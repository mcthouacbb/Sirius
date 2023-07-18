#include <iostream>
#include <chrono>
#include <fstream>
#include <charconv>
#include <vector>
#include <cstring>
#include <chrono>

#include "board.h"
#include "attacks.h"
#include "movegen.h"
#include "comm/move.h"
#include "comm/fen.h"
#include "eval/eval.h"
#include "search.h"

void printBoard(const Board& board)
{
	std::cout << board.stringRep() << std::endl;
	std::cout << "GamePly: " << board.gamePly() << std::endl;
	std::cout << "HalfMoveClock: " << board.halfMoveClock() << std::endl;
	std::cout << "CastlingRights: " << board.castlingRights() << std::endl;
	std::cout << "Side to move: " << (board.currPlayer() == Color::WHITE ? "WHITE" : "BLACK") << std::endl;
	if (board.epSquare() != 0)
		std::cout << "Ep square: " << static_cast<char>((board.epSquare() & 7) + 'a') << static_cast<char>((board.epSquare() >> 3) + '1') << std::endl;
	else
		std::cout << "Ep square: N/A" << std::endl;
}

template<bool print>
uint64_t perft(Board& board, int depth)
{
	if (depth == 0)
		return 1;
	Move moves[256];
	Move* end = genMoves<MoveGenType::LEGAL>(board, moves, calcCheckInfo(board, board.currPlayer()));
	if (depth == 1 && !print)
		return end - moves;

	uint64_t count = 0;
	BoardState state;
	for (Move* it = moves; it != end; it++)
	{
		const auto& move = *it;
		board.makeMove(move, state);
		uint64_t sub = perft<false>(board, depth - 1);
		if (print)
			std::cout << comm::convMoveToPCN(move) << ": " << sub << std::endl;
		count += sub;
		board.unmakeMove(move, state);
	}
	return count;
}

void testSAN(Board& board, int depth)
{
	Move moves[256];
	Move* end = genMoves<MoveGenType::LEGAL>(board, moves, calcCheckInfo(board, board.currPlayer()));

	for (Move* it = moves; it != end; it++)
	{
		std::string str = comm::convMoveToSAN(board, moves, end, *it);
		auto [move, strEnd] = comm::findMoveFromSAN(board, moves, end, str.c_str());
		if (move != it)
		{
			std::cerr << board.stringRep() << std::endl;
			std::cout << str << ' ' << comm::convMoveToPCN(*it) << std::endl;
			std::cerr << "No match " << it - moves << std::endl;
			exit(1);
		}

		if (strEnd != str.c_str() + str.length())
		{
			std::cerr << board.stringRep() << std::endl;
			std::cerr << str << ' ' << comm::convMoveToPCN(*it) << std::endl;
			std::cerr << "String wrong" << std::endl;
			exit(1);
		}
	}

	if (depth == 0)
		return;

	BoardState state;
	for (Move* it = moves; it != end; it++)
	{
		const auto& move = *it;
		board.makeMove(move, state);
		testSAN(board, depth - 1);
		board.unmakeMove(move, state);
	}
}

void testQuiescence(Board& board, int depth)
{
	if (depth == 0)
	{
		Move captures[256];
		Move* end = genMoves<MoveGenType::CAPTURES>(board, captures, calcCheckInfo(board, board.currPlayer()));

		for (Move* it = captures; it != end; it++)
		{
			if (it->type() != MoveType::ENPASSANT && !(board.getPieceAt(it->dstPos())))
			{
				throw std::runtime_error("Not capture");
			}
		}
		return;
	}
	Move moves[256];
	Move* end = genMoves<MoveGenType::LEGAL>(board, moves, calcCheckInfo(board, board.currPlayer()));

	BoardState state;
	for (Move* it = moves; it != end; it++)
	{
		const auto& move = *it;
		board.makeMove(move, state);
		testQuiescence(board, depth - 1);
		board.unmakeMove(move, state);
	}
}

struct PerftTest
{
	std::string fen;
	uint64_t results[6];
};

void runTests(Board& board, bool fast)
{
	std::ifstream file("res/perft_tests.txt");
	if (file.is_open())
	{
		std::cout << "open" << std::endl;
	}
	else
	{
		std::cout << "closed" << std::endl;
	}
	std::string line;
	std::vector<PerftTest> tests;
	while (std::getline(file, line))
	{
		PerftTest test;
		std::fill(std::begin(test.results), std::end(test.results), UINT64_MAX);
		int i = 0;
		while (line[i] != ';')
			i++;
		test.fen = line.substr(0, i);

		std::from_chars(line.data() + i + 4, line.data() + line.size(), test.results[line[i + 2] - '1']);

		while (true)
		{
			size_t idx = line.find(';', i + 1);
			if (idx == std::string::npos)
				break;
			i = static_cast<int>(idx);
			std::from_chars(line.data() + i + 4, line.data() + line.size(), test.results[line[i + 2] - '1']);
		}
		tests.push_back(test);
	}
	/*for (auto& test : tests)
	{
		std::cout << "FEN: " << test.fen << std::endl;
		for (int i = 0; i < 6; i++)
		{
			if (test.results[i] != UINT64_MAX)
			{
				std::cout << "DEPTH: " << (i + 1) << " P: " << test.results[i] << std::endl;
			}
		}
	}*/

/*
TEST: 8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1
    Passed: depth 4
    Failed: depth 5, Expected: 674624, got: 674543
    Failed: depth 6, Expected: 11030083, got: 11027209

*/

	uint32_t failCount = 0;
	uint64_t totalNodes = 0;

	auto t1 = std::chrono::steady_clock::now();
	for (size_t i = 0; i < tests.size(); i++)
	{
		const auto& test = tests[i];
		board.setToFen(test.fen);
		std::cout << "TEST: " << test.fen << std::endl;
		for (int j = 0; j < 6; j++)
		{
			if (test.results[j] == UINT64_MAX)
				continue;
			if (fast && test.results[j] > 100000000)
			{
				std::cout << "\tSkipped: depth " << j + 1 << std::endl;
				continue;
			}
			uint64_t nodes = perft<false>(board, j + 1);
			totalNodes += nodes;
			if (nodes == test.results[j])
			{
				std::cout << "\tPassed: depth " << j + 1 << std::endl;
			}
			else
			{
				std::cout << "\tFailed: depth " << j + 1 << ", Expected: " << test.results[j] << ", got: " << nodes << std::endl;
				failCount++;
			}
		}
	}
	auto t2 = std::chrono::steady_clock::now();
	std::cout << "Failed: " << failCount << std::endl;
	std::cout << "Nodes: " << totalNodes << std::endl;
	std::cout << "Time: " << (t2 - t1).count() << std::endl;
}

void testSANFind(const Board& board, Move* begin, Move* end, int len)
{
	static constexpr char chars[25] = {
		'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
		'1', '2', '3', '4', '5', '6', '7', '8',
		'K', 'Q', 'R', 'B', 'N', 'q', 'r', 'n',
		'x'
	};
	static constexpr uint64_t charCount = 25;
	char* buf = new char[len + 1];
	buf[len] = '\0';

	uint64_t maxIdx = 1;

	for (int i = 0; i < len; i++)
	{
		maxIdx *= charCount;
	}
	for (uint64_t i = 0; i < maxIdx; i++)
	{
		uint64_t tmp = i;
		for (int j = 0; j < len; j++)
		{
			buf[j] = chars[tmp % charCount];
			tmp /= charCount;
		}

		// std::cout << buf << '\n';

		comm::findMoveFromSAN(board, begin, end, buf);
	}
	delete[] buf;
}

enum class Command
{
	SET_POSITION,
	MAKE_MOVE,
	UNDO_MOVE,
	PRINT_BOARD,
	STATIC_EVAL,
	SEARCH,
	QUIT
};

const char* parseCommand(const char* str, Command& command)
{
	switch (str[0])
	{
		case 'p':
			switch (str[1])
			{
				case 'o':
					if (strncmp(str + 2, "sition ", 7) == 0)
					{
						command = Command::SET_POSITION;
						return str + 9;
					}
					return nullptr;
				case 'r':
					if (strncmp(str + 2, "int", 3) == 0)
					{
						command = Command::PRINT_BOARD;
						return str + 5;
					}
					return nullptr;
				default:
					return nullptr;
			}
		case 'm':
			if (strncmp(str + 1, "ove ", 4) == 0)
			{
				command = Command::MAKE_MOVE;
				return str + 5;
			}
			return nullptr;
		case 'u':
			if (strncmp(str + 1, "ndo", 3) == 0)
			{
				command = Command::UNDO_MOVE;
				return str + 4;
			}
			return nullptr;
		case 's':
			if (strncmp(str + 1, "earch ", 6) == 0)
			{
				command = Command::SEARCH;
				return str + 7;
			}
		case 'e':
			if (strncmp(str + 1, "val", 3) == 0)
			{
				command = Command::STATIC_EVAL;
				return str + 4;
			}
			return nullptr;
		case 'q':
			if (strncmp(str + 1, "uit", 3) == 0)
			{
				command = Command::QUIT;
				return str + 4;
			}
			return nullptr;
		default:
			return nullptr;
	}
}

struct State
{
	Board* board;
	std::vector<BoardState> prevStates;
	std::vector<Move> prevMoves;
	Move moves[256];
	Move* end;
};

void setPosition(State& state, std::string_view params)
{
	if (strncmp(params.data(), "fen ", 4) == 0)
	{
		if (!comm::isValidFen(params.data() + 4))
		{
			std::cout << "Invalid fen string" << std::endl;
			return;
		}
		state.board->setToFen(params.data() + 4);
	}
	else if (strncmp(params.data(), "startpos\0", 9) == 0)
	{
		state.board->setToFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	}
	else
	{
		std::cout << "Invalid position" << std::endl;
		return;
	}
	state.prevStates.clear();
	state.prevMoves.clear();
	state.end = genMoves<MoveGenType::LEGAL>(*state.board, state.moves, calcCheckInfo(*state.board, state.board->currPlayer()));
}

void makeMove(State& state, std::string_view params)
{
	auto [move, strEnd] = comm::findMoveFromSAN(*state.board, state.moves, state.end, params.data());

	if (move == nullptr)
	{
		std::cout << "Invalid move" << std::endl;
		return;
	}

	if (move == state.end)
	{
		std::cout << "No move found" << std::endl;
		return;
	}

	if (move == state.end + 1)
	{
		std::cout << "Move is ambiguous" << std::endl;
		return;
	}

	state.prevStates.emplace_back(BoardState());
	state.board->makeMove(*move, state.prevStates.back());
	state.prevMoves.push_back(*move);

	state.end = genMoves<MoveGenType::LEGAL>(*state.board, state.moves, calcCheckInfo(*state.board, state.board->currPlayer()));
}

void undoMove(State& state, std::string_view)
{
	if (state.prevMoves.empty())
	{
		std::cout << "No move to undo" << std::endl;
		return;
	}

	state.board->unmakeMove(state.prevMoves.back(), state.prevStates.back());
	state.prevStates.pop_back();
	state.prevMoves.pop_back();

	state.end = genMoves<MoveGenType::LEGAL>(*state.board, state.moves, calcCheckInfo(*state.board, state.board->currPlayer()));
}

void staticEval(const Board& board)
{
	std::cout << "Eval: " << eval::evaluate(board) << std::endl;
	std::cout << "Phase: " << board.evalState().phase << std::endl;
	std::cout << "Eval midgame:\n";
	std::cout << "\tMaterial:\n";
	std::cout << "\t\tWhite: " << board.evalState().materialMG[0] << '\n';
	std::cout << "\t\tBlack: " << board.evalState().materialMG[1] << '\n';
	std::cout << "\tPiece Square Tables:\n";
	std::cout << "\t\tWhite: " << board.evalState().psqtMG[0] << '\n';
	std::cout << "\t\tBlack: " << board.evalState().psqtMG[1] << '\n';
	std::cout << "Eval endgame:\n";
	std::cout << "\tMaterial:\n";
	std::cout << "\t\tWhite: " << board.evalState().materialEG[0] << '\n';
	std::cout << "\t\tBlack: " << board.evalState().materialEG[1] << '\n';
	std::cout << "\tPiece Square Tables:\n";
	std::cout << "\t\tWhite: " << board.evalState().psqtEG[0] << '\n';
	std::cout << "\t\tBlack: " << board.evalState().psqtEG[1] << '\n';
}

void searchCommand(Search& search, std::string_view params)
{
	int depth;
	auto result = std::from_chars(params.data(), params.data() + params.size(), depth);

	if (result.ec != std::errc())
	{
		std::cout << "Depth must be a valid integer" << std::endl;
		return;
	}

	if (depth <= 0)
	{
		std::cout << "Depth must be greater than 0" << std::endl;
		return;
	}

	auto t1 = std::chrono::steady_clock::now();
	int eval = search.iterDeep(depth);
	auto t2 = std::chrono::steady_clock::now();

	auto time = std::chrono::duration_cast<std::chrono::duration<float>>(t2 - t1);

	std::cout << "Time: " << time.count() << std::endl;
	std::cout << "Eval: " << eval << std::endl;
}

bool execCommand(State& state, Search& search, const std::string& str)
{
	Command command;
	const char* params = parseCommand(str.c_str(), command);
	if (params == nullptr)
	{
		std::cout << "Invalid command" << std::endl;
		return false;
	}

	switch (command)
	{
		case Command::SET_POSITION:
			std::cout << "Set position: " << params << std::endl;
			setPosition(state, std::string_view(params, str.c_str() + str.size()));
			break;
		case Command::MAKE_MOVE:
			makeMove(state, std::string_view(params, str.c_str() + str.size()));
			std::cout << "Make move: " << params << std::endl;
			break;
		case Command::UNDO_MOVE:
			undoMove(state, std::string_view(params, str.c_str() + str.size()));
			std::cout << "Undo move: " << params << std::endl;
			break;
		case Command::PRINT_BOARD:
			std::cout << "Print board: " << params << std::endl;
			printBoard(*state.board);
			break;
		case Command::STATIC_EVAL:
			std::cout << "Static eval: " << params << std::endl;
			staticEval(*state.board);
			break;
		case Command::SEARCH:
			std::cout << "Search: " << params << std::endl;
			searchCommand(search, std::string_view(params, str.c_str() + str.size()));
			break;
		case Command::QUIT:
			std::cout << "Quitting" << std::endl;
			return true;
	}
	return false;
}

int main()
{
	attacks::init();
	std::cout << "Hello World!" << std::endl;

	// board.setToFen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");

	// testQuiescence(board, 3);
	// std::cout << "yay" << std::endl;

	std::string str;
	std::getline(std::cin, str);

	if (str == "uci")
	{
		// TODO: uci stuff
		return 0;
	}

	Board board;

	State state;
	state.board = &board;
	state.end = genMoves<MoveGenType::LEGAL>(board, state.moves, calcCheckInfo(*state.board, state.board->currPlayer()));

	Search search(board);

	if (execCommand(state, search, str))
		return 0;

	for (;;)
	{
		std::getline(std::cin, str);
		if (execCommand(state, search, str))
			return 0;
	}
	return 0;
}
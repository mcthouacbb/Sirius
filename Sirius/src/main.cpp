#include <iostream>
#include <chrono>
#include <fstream>
#include <charconv>
#include <vector>
#include <cstring>
#include <chrono>
#include <deque>
#include <tuple>
#include <sstream>
#include <random>

#include "board.h"
#include "attacks.h"
#include "movegen.h"
#include "comm/move.h"
#include "comm/fen.h"
#include "eval/eval.h"
#include "search.h"
#include "misc.h"
#include "comm/cmdline.h"

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

	std::cout << "Fen: " << board.fenStr() << std::endl;
}

enum class Command
{
	SET_POSITION,
	MAKE_MOVE,
	UNDO_MOVE,
	PRINT_BOARD,
	STATIC_EVAL,
	QUIESCENCE_EVAL,
	SEARCH,
	RUN_TESTS,
	PERFT,
	SET_CLOCK,
	BOOK
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
				case 'e':
					if (strncmp(str + 2, "rft ", 4) == 0)
					{
						command = Command::PERFT;
						return str + 6;
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
			return nullptr;
		case 'e':
			if (strncmp(str + 1, "val", 3) == 0)
			{
				command = Command::STATIC_EVAL;
				return str + 4;
			}
			return nullptr;
		case 't':
			if (strncmp(str + 1, "ests", 4) == 0)
			{
				command = Command::RUN_TESTS;
				return str + 5;
			}
			return nullptr;
		case 'q':
			if (strncmp(str + 1, "eval", 4) == 0)
			{
				command = Command::QUIESCENCE_EVAL;
				return str + 5;
			}
			return nullptr;
		case 'c':
			if (strncmp(str + 1, "lock ", 5) == 0)
			{
				command = Command::SET_CLOCK;
				return str + 6;
			}
			return nullptr;
		case 'b':
			if (strncmp(str + 1, "ook", 3) == 0)
			{
				command = Command::BOOK;
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
	// use deque to avoid invalidation of pointers
	std::deque<BoardState> prevStates;
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
	auto [move, strEnd] = comm::findMoveFromPCN(state.moves, state.end, params.data());

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
	std::cout << "Eval endgame:\n";
	std::cout << "\tMaterial:\n";
	std::cout << "\t\tWhite: " << board.evalState().materialEG[0] << '\n';
	std::cout << "\t\tBlack: " << board.evalState().materialEG[1] << '\n';
	std::cout << "\tPiece Square Tables:\n";
}

void quiescenceEval(Search& search, std::string_view)
{
	int eval = search.qsearch(eval::NEG_INF, eval::POS_INF);
	std::cout << "Quiescence eval: " << eval << std::endl;
}

void searchCommand(Search& search, std::string_view params)
{
	int depth;
	auto [ptr, ec] = std::from_chars(params.data(), params.data() + params.size(), depth);

	if (ec != std::errc())
	{
		std::cout << "Invalid depth" << std::endl;
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

	std::cout << "Eval: " << eval << std::endl;
	std::cout << std::endl;
}

void setClock(Search& search, std::string_view params)
{
	uint32_t clock;
	auto [ptr, ec] = std::from_chars(params.data(), params.data() + params.size(), clock);

	if (ec != std::errc())
	{
		std::cout << "Invalid number of milliseconds for clock" << std::endl;
		return;
	}

	uint32_t increment;
	auto result = std::from_chars(ptr + 1, params.data() + params.size(), increment);

	if (result.ec != std::errc())
	{
		std::cout << "Invalid number of milliseconds for increment" << std::endl;
		return;
	}

	search.setTime(Duration(clock), Duration(increment));
}

int main()
{
	attacks::init();

	std::string mode;
	std::getline(std::cin, mode);
	if (mode == "cmdline")
	{
		comm::CmdLine cmdLine;
		for (;;)
		{
			std::string cmd;
			std::getline(std::cin, cmd);
			if (cmd == "quit")
				break;
			cmdLine.execCommand(cmd);
		}
	}
	else
	{
		std::cout << "Unrecognized mode" << std::endl;
	}
	return 0;
	/*Board board;
	// board.setToFen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");

	// testQuiescence(board, 3);
	// std::cout << "yay" << std::endl;

	State state;
	state.board = &board;
	state.end = genMoves<MoveGenType::LEGAL>(board, state.moves);

	Search search(board);
	// default to 3m|1s blitz
	search.setTime(Duration(180000), Duration(1000));

	std::string str;
	for (;;)
	{
		std::getline(std::cin, str);
		Command command;
		const char* params = parseCommand(str.c_str(), command);
		if (params == nullptr)
		{
			std::cout << "Invalid command: " << str << std::endl;
			continue;
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
				printBoard(board);
				break;
			case Command::STATIC_EVAL:
				std::cout << "Static eval: " << params << std::endl;
				staticEval(board);
				break;
			case Command::SEARCH:
				std::cout << "Search: " << params << std::endl;
				searchCommand(search, std::string_view(params, str.c_str() + str.size()));
				break;
			case Command::QUIESCENCE_EVAL:
				std::cout << "QEval: " << params << std::endl;
				quiescenceEval(search, std::string_view(params, str.c_str() + str.size()));
				break;
			case Command::RUN_TESTS:
				std::cout << "Tests: " << params << std::endl;
				runTests(board, false);
				break;
			case Command::PERFT:
			{
				std::cout << "Perft: " << params << std::endl;
				int depth;
				auto [ptr, ec] = std::from_chars(params, str.c_str() + str.size(), depth);
				if (ec != std::errc())
				{
					std::cout << "Invalid depth" << std::endl;
					break;
				}
				auto t1 = std::chrono::steady_clock::now();
				uint64_t nodes = perft<true>(board, depth);
				auto t2 = std::chrono::steady_clock::now();
				std::cout << (t2 - t1).count() << std::endl;
				std::cout << "Nodes: " << nodes << std::endl;
				break;
			}
			case Command::SET_CLOCK:
				std::cout << "Set clock: " << params << std::endl;
				setClock(search, params);
				break;
			case Command::BOOK:
				std::cout << "Book: " << params << std::endl;
				const std::vector<BookEntry>* entries = book.lookup(board.zkey());
				if (strncmp(params, " rand", 5) == 0)
				{
					if (!entries)
						std::cout << "No moves in book found" << std::endl;
					else
					{
						auto dist = std::uniform_int_distribution<int>(0, entries->size() - 1);
						int index = dist(rng);
						std::cout << comm::convMoveToPCN((*entries)[index].move) << std::endl;
					}
				}
				else
				{
					if (!entries)
						std::cout << "No moves in book found" << std::endl;
					else
					{
						for (const auto& entry : *entries)
						{
							std::cout << comm::convMoveToPCN(entry.move) << ' ';
						}
						std::cout << std::endl;
					}
				}
				break;
		}
	}
	return 0;*/
}
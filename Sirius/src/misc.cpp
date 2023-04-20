#include "misc.h"
#include "comm/move.h"
#include "movegen.h"
#include <chrono>
#include <charconv>
#include <vector>
#include <fstream>

template<bool print>
uint64_t perft(Board& board, int depth)
{
	if (depth == 0)
		return 1;
	Move moves[256];
	Move* end = genMoves<MoveGenType::LEGAL>(board, moves);
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
		board.unmakeMove(move);
	}
	return count;
}

template uint64_t perft<true>(Board& board, int depth);
template uint64_t perft<false>(Board& board, int depth);

template<bool print>
uint64_t testGivesCheck(Board& board, int depth)
{
	if (depth == 0)
		return 1;
	Move moves[256];
	Move* end = genMoves<MoveGenType::LEGAL>(board, moves);
	if (depth == 1 && !print)
		return end - moves;

	uint64_t count = 0;
	BoardState state;
	for (Move* it = moves; it != end; it++)
	{
		const auto& move = *it;
		bool givesCheck = board.givesCheck(move);
		board.makeMove(move, state);
		bool inCheck = board.checkers() != 0;
		if (givesCheck != inCheck)
		{
			std::cout << comm::convMoveToPCN(move) << std::endl;
			board.unmakeMove(move);
			// printBoard(board);
			throw std::runtime_error("WHAT?");
		}
		uint64_t sub = perft<false>(board, depth - 1);
		if (print)
			std::cout << comm::convMoveToPCN(move) << ": " << sub << std::endl;
		count += sub;
		board.unmakeMove(move);
	}
	return count;
}

template uint64_t testGivesCheck<true>(Board& board, int depth);
template uint64_t testGivesCheck<false>(Board& board, int depth);

void testSAN(Board& board, int depth)
{
	Move moves[256];
	Move* end = genMoves<MoveGenType::LEGAL>(board, moves);

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
		board.unmakeMove(move);
	}
}

void testQuiescence(Board& board, int depth)
{
	if (depth == 0)
	{
		Move captures[256];
		Move* end = genMoves<MoveGenType::CAPTURES>(board, captures);

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
	Move* end = genMoves<MoveGenType::LEGAL>(board, moves);

	BoardState state;
	for (Move* it = moves; it != end; it++)
	{
		const auto& move = *it;
		board.makeMove(move, state);
		testQuiescence(board, depth - 1);
		board.unmakeMove(move);
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
			i = idx;
			std::from_chars(line.data() + i + 4, line.data() + line.size(), test.results[line[i + 2] - '1']);
		}
		tests.push_back(test);
	}

	uint32_t failCount = 0;
	uint64_t totalNodes = 0;

	auto t1 = std::chrono::steady_clock::now();
	for (int i = 0; i < tests.size(); i++)
	{
		const auto& test = tests[i];
		board.setToFen(test.fen);
		std::cout << "TEST: " << test.fen << std::endl;
		for (int i = 0; i < 6; i++)
		{
			if (test.results[i] == UINT64_MAX)
				continue;
			if (fast && test.results[i] > 100000000)
			{
				std::cout << "\tSkipped: depth " << i + 1 << std::endl;
				continue;
			}
			// uint64_t nodes = testGivesCheck<false>(board, i + 1);
			uint64_t nodes = perft<false>(board, i + 1);
			totalNodes += nodes;
			if (nodes == test.results[i])
			{
				std::cout << "\tPassed: depth " << i + 1 << std::endl;
			}
			else
			{
				std::cout << "\tFailed: depth " << i + 1 << ", Expected: " << test.results[i] << ", got: " << nodes << std::endl;
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

	for (uint64_t i = 0; i < len; i++)
	{
		maxIdx *= charCount;
	}
	for (uint64_t i = 0; i < maxIdx; i++)
	{
		uint64_t tmp = i;
		for (int i = 0; i < len; i++)
		{
			buf[i] = chars[tmp % charCount];
			tmp /= charCount;
		}

		// std::cout << buf << '\n';

		comm::findMoveFromSAN(board, begin, end, buf);
	}
	delete[] buf;
}
#include <iostream>
#include <chrono>
#include <fstream>
#include <charconv>
#include <vector>

#include "board.h"
#include "attacks.h"
#include "movegen.h"

std::string moveStr(Move move)
{
	std::string str(4 + (move.type() == MoveType::PROMOTION), ' ');
	str[0] = static_cast<char>((move.srcPos() & 7) + 'a');
	str[1] = static_cast<char>((move.srcPos() >> 3) + '1');
	str[2] = static_cast<char>((move.dstPos() & 7) + 'a');
	str[3] = static_cast<char>((move.dstPos() >> 3) + '1');
	if (move.type() == MoveType::PROMOTION)
	{
		const char promoChars[4] = {'q', 'r', 'b', 'n'};
		str[4] = promoChars[(static_cast<int>(move.promotion()) >> 14)];
	}
	return str;
}

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
			std::cout << moveStr(move) << ": " << sub << std::endl;
		count += sub;
		board.unmakeMove(move, state);
	}
	return count;
}

struct PerftTest
{
	std::string fen;
	uint64_t results[6];
};

void runTests(Board& board)
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
		std::cout << line << std::endl;
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
	for (uint32_t i = 0; i < tests.size(); i++)
	{
		const auto& test = tests[i];
		board.setToFen(test.fen);
		std::cout << "TEST: " << test.fen << std::endl;
		for (int j = 0; j < 6; j++)
		{
			if (test.results[j] == UINT64_MAX)
				continue;
			uint64_t nodes = perft<false>(board, i + 1);
			if (nodes == test.results[j])
			{
				std::cout << "\tPassed: depth " << i + 1 << std::endl;
			}
			else
			{
				std::cout << "\tFailed: depth " << i + 1 << ", Expected: " << test.results[j] << ", got: " << nodes << std::endl;
				failCount++;
			}
		}
	}
	std::cout << "Failed: " << failCount << std::endl;
}

int main()
{
	attacks::init();
	std::cout << "Hello World!" << std::endl;
	Board board;
	// board.setToFen("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");

	runTests(board);
	// board.setToFen("8/8/3p4/1Pp4r/1K5k/5p2/4P1P1/1R6 w - c6 0 3");
	// std::cout << perft<true>(board, 1) << std::endl;
	
	// board.setToFen("8/2p5/3p4/KP5r/1R2Pp1k/8/6P1/8 b - e3 0 1");
	// runTests(board);
	// board.setToFen("rnb1kbnr/ppp2ppp/8/3pp3/3PP3/5N2/PPP1QPPR/RNB1KB2 b Qkq - 1 5");
	// board.setToFen("rnb1kbnr/ppp2ppp/8/3pp3/3PP3/5N2/PPP2PPR/RNBQKB2 w Qkq d6 0 5");
	// board.setToFen("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8");
	// board.setToFen("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1");
	// board.printDbg();
	// Move moves[256];
	// Move* end = genMoves<MoveGenType::LEGAL>(board, moves);
	// std::cout << end - moves << std::endl;
	// std::cout << perft<true>(board, 3) << std::endl;
	// BoardState state;
	// std::cout << perft<true>(board, 1) << std::endl;
	// board.makeMove(Move(14, 22, MoveType::NONE), state);
	// board.makeMove(Move(11, 19, MoveType::NONE), state);
	// board.makeMove(Move(62, 45, MoveType::NONE), state);
	// board.setToFen("rnbqkb1r/pppppppp/8/8/4n3/3P4/PPPKPPPP/RNBQ1BNR w kq - 3 3");
	// board.setToFen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
	/*Move moves[256];
	Move* end = genMoves<MoveGenType::LEGAL>(board, moves);
	for (Move* it = moves; it != end; it++)
	{
		const auto& move = *it;
		std::cout << moveStr(move) << std::endl;
	}*/
	// std::cout << perft<true>(board, 3) << std::endl;
	// printBoard(board);
	// runTests(board);
	// printBB(0x70);
	// printBB(0x1C);
	// runTests(board);
	// auto t1 = std::chrono::steady_clock::now();
	// board.setToFen("rnb1kbnr/pppp1ppp/4pq2/8/8/5P2/PPPPPKPP/RNBQ1BNR w kq - 2 3");
	// std::cout << perft<true>(board, 6) << std::endl;
	// auto t2 = std::chrono::steady_clock::now();
	// std::cout << (t2 - t1).count() << std::endl;
	// board.makeMove(Move(10, 18, MoveType::NONE), state);
	// board.makeMove(Move(52, 44, MoveType::NONE), state);
	// board.makeMove(Move(11, 19, MoveType::NONE), state);
	// board.makeMove(Move(61, 25, MoveType::NONE), state);
	// std::cout << perft<true>(board, 1);
	// board.makeMove(moves[0], state);
	// board.unmakeMove(moves[0], state);
	// board.makeMove(Move(10, 18, MoveType::NONE), state);
	// board.makeMove(Move(51, 43, MoveType::NONE), state);
	// board.makeMove(Move(3, 24, MoveType::NONE), state);
	// printBoard(board);
	// std::cout << perft<true>(board, 2) << std::endl;
	// board.printDbg();
	// std::cout << perft<true>(board, 3) << std::endl;
	// BoardState state;
	// board.makeMove(Move(), state);
	return 0;
}

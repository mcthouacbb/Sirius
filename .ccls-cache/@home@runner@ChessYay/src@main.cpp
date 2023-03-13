#include <iostream>
// #include <fmt/format.h>
#include <chrono>
#include <vector>
#include "board.h"
#include "movegen.h"
#include "attacks.h"
#include "eval.h"

struct PerftResult
{
	uint64_t nodeCount;
	char name[5];
};
uint64_t perft(Board& board, uint32_t depth);

uint64_t perftMoves(Board& board, uint32_t depth, std::vector<PerftResult>& results)
{
	if (depth == 0)
		return 1;
	Move moves[256];
	Move* end = genMoves<MoveGenType::LEGAL>(board, moves);

	uint64_t nodeCount = 0;
	// if (depth == 1)
		// return moves.size();
	results.resize(end - moves);
	for (uint32_t i = 0; i < end - moves; i++)
	//for (size_t i = 0; i < ; i++)
	{
		results[i].name[0] = (moves[i].srcPos() & 7) + 'a';
		results[i].name[1] = (moves[i].srcPos() >> 3) + '1';
		results[i].name[2] = (moves[i].dstPos() & 7) + 'a';
		results[i].name[3] = (moves[i].dstPos() >> 3) + '1';
		if (moves[i].type() == MoveType::PROMOTION || moves[i].type() == MoveType::CAPTURE_PROMOTION)
		{
			extern char blackPieceChars[6];
			results[i].name[4] = blackPieceChars[static_cast<int>(moves[i].dstPieceType())];
		}
		else
		{
			results[i].name[4] = '\0';
		}
		if (depth > 1)
		{
			board.makeMove(moves[i]);
			nodeCount += results[i].nodeCount = perft(board, depth - 1);
			board.unmakeMove(moves[i]);
		}
		else
		{
			nodeCount += results[i].nodeCount = 1;
		}
	}
	return nodeCount;
}

uint64_t perft(Board& board, uint32_t depth)
{
	if (depth == 0)
		return 1;
	Move moves[256];
	Move* end = genMoves<MoveGenType::LEGAL>(board, moves);

	//if (depth == 1)
		//return end - moves;

	uint64_t count = 0;

	for (Move* it = moves; it != end; it++)
	{
		const auto& move = *it;
		uint32_t srcPos = move.srcPos();
		uint32_t dstPos = move.dstPos();
		board.makeMove(move);
		count += perft(board, depth - 1);
		board.unmakeMove(move);
	}
	return count;
}

constexpr int CHECKMATE = -1000000;
constexpr int NEG_INF = -99999999;
constexpr int POS_INF = 99999999;

void printMove(Move move)
{
	std::cout << static_cast<char>(((move.srcPos() & 7) + 'a'));
	std::cout << static_cast<char>(((move.srcPos() >> 3) + '1'));
	std::cout << static_cast<char>(((move.dstPos() & 7) + 'a'));
	std::cout << static_cast<char>(((move.dstPos() >> 3) + '1'));
	if (move.type() == MoveType::PROMOTION || move.type() == MoveType::CAPTURE_PROMOTION)
	{
		extern char blackPieceChars[6];
		std::cout << blackPieceChars[static_cast<int>(move.dstPieceType())];
	}
}

int searchInner(Board& board, uint32_t depth, uint32_t rootDepth, int alpha, int beta);

int search(Board& board, uint32_t depth, Move& outMove)
{
	Move moves[256];
	Move* end = genMoves<MoveGenType::LEGAL>(board, moves);

	if (moves == end)
	{
		if (board.isInCheck())
			return CHECKMATE;
		return 0;
	}

	Move* bestMove;
	int alpha = NEG_INF;
	for (Move* m = moves; m != end; m++)
	{
		const auto& move = *m;
		board.makeMove(move);
		int moveEval = -searchInner(board, depth - 1, 1, NEG_INF, -alpha);
		board.unmakeMove(move);
		if (moveEval > alpha)
		{
			alpha = moveEval;
			bestMove = m;
		}
	}

	outMove = *bestMove;

	return alpha;
}

int searchInner(Board& board, uint32_t depth, uint32_t rootDepth, int alpha, int beta)
{
	if (depth == 0)
		return evaluate(board);
	Move moves[256];
	Move* end = genMoves<MoveGenType::LEGAL>(board, moves);

	if (moves == end)
	{
		if (board.isInCheck())
			return CHECKMATE + rootDepth;
		return 0;
	}

	// int eval = INT32_MIN;
	for (Move* m = moves; m != end; m++)
	{
		const auto& move = *m;
		board.makeMove(move);
		int moveEval = -searchInner(board, depth - 1, rootDepth + 1, -beta, -alpha);
		board.unmakeMove(move);
		if (moveEval >= beta)
			return beta;
		if (moveEval > alpha)
			alpha = moveEval;
		// eval = std::max(eval, moveEval);
	}

	return alpha;
}

int main()
{
	genAttackData();
	Board board;
	// Board board("6k1/3b3r/1p1p4/p1n2p2/1PPNpq2/P3Q1p1/1R1RB1P1/5K2 w - - 0 2");
	// Board board("6k1/3b4/1p1p4/p1n2p2/1PPNpq2/P3Q1p1/1R1RB1P1/4K2r w - - 2 3");
	/*board.makeMove(Move(MoveType::DOUBLE_PUSH, 10, 18, PieceType::PAWN));
	board.makeMove(Move(MoveType::NONE, 57, 40, PieceType::KNIGHT));
	board.makeMove(Move(MoveType::NONE, 3, 17, PieceType::QUEEN));
	board.makeMove(Move(MoveType::NONE, 40, 25, PieceType::KNIGHT));*/
	std::cout << board.getString() << std::endl;

	// printBB(ranks[1] | ranks[0] | ranks[6] | ranks[7]);
	// printBB(getRookAttacks(0, ranks[1] | ranks[0] | ranks[6] | ranks[7]));

	/*uint64_t nodes;
	std::vector<PerftResult> results;

	auto t1 = std::chrono::high_resolution_clock::now();
	nodes = perftMoves(board, 6, results);
	auto t2 = std::chrono::high_resolution_clock::now();

	std::cout << "total: " << nodes << std::endl;
	for (const auto& result : results)
	{
		std::cout.write(result.name, 4 + (result.name[4] != '\0')) << ": " << result.nodeCount << '\n';
	}
	std::cout << "time(ns): " << (t2 - t1).count() << std::endl*/

	std::cout << evaluate(board) << std::endl;
	
	auto t1 = std::chrono::high_resolution_clock::now();
	Move bestMove;
	int eval = search(board, 7, bestMove);
	auto t2 = std::chrono::high_resolution_clock::now();
	std::cout << eval << std::endl;
	std::cout << bestMove.srcPos() << ' ' << bestMove.dstPos() << std::endl;
	std::cout << "time(ns): " << (t2 - t1).count() << std::endl;
	// std::cin.get();
	// std::cin.get();
	// std::cin.get();
	// std::cin.get();
	// std::cin.get();
	// std::cout << moves.size() << std::endl;
	// std::cout << "Hello World!" << std::endl;
	return 0;
}
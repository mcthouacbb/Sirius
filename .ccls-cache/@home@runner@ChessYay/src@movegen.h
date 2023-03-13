#include "move.h"
#include "board.h"

/*#include <vector>

std::vector<Move> genMoves(const Board& board);
std::vector<Move> genMovesWhite(
	const Board& board,
	BitBoard checkBB,
	BitBoard checkingPieces,
	BitBoard pinnedPieces,
	BitBoard blockMask,
	BitBoard captureMask);
std::vector<Move> genMovesBlack(
	const Board& board,
	BitBoard checkBB,
	BitBoard checkingPieces,
	BitBoard pinnedPieces,
	BitBoard blockMask,
	BitBoard captureMask);*/

/*#include <array>

struct MoveList
{
	std::array<Move, 256> moves;
	uint32_t moveCount;

	auto begin()
	{
		return moves.begin();
	}

	auto end()
	{
		return moves.end();
	}
};*/
enum class MoveGenType
{
	LEGAL,
	CAPTURES
};
template<MoveGenType type>
Move* genMoves(Board& board, Move* moves);

template<MoveGenType type>
Move* genMovesWhite(
	const Board& board,
	Move* moves,
	BitBoard checkBB,
	BitBoard checkingPieces,
	BitBoard pinnedPieces,
	BitBoard blockMask,
	BitBoard captureMask);

template<MoveGenType type>
Move* genMovesBlack(
	const Board& board,
	Move* moves,
	BitBoard checkBB,
	BitBoard checkingPieces,
	BitBoard pinnedPieces,
	BitBoard blockMask,
	BitBoard captureMask);
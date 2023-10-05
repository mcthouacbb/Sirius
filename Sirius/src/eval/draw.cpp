#include "draw.h"

namespace eval
{

bool isImmediateDraw(const Board& board)
{
	BitBoard nonMinorPcs =
		board.getPieces(PieceType::QUEEN) |
		board.getPieces(PieceType::ROOK) |
		board.getPieces(PieceType::PAWN);

	if (nonMinorPcs != 0)
		return false;

	switch (getPopcnt(board.getAllPieces()))
	{
		case 2:
			return true;
		case 3:
			return true;
		case 4:
		{
			BitBoard bishops = board.getPieces(PieceType::BISHOP);
			if (getPopcnt(bishops) == 2 && (getPopcnt(bishops & LIGHT_SQUARES) == 2 || (bishops & LIGHT_SQUARES) == 0))
				return true;
			return false;
		}
		default:
			return false;
	}
}

bool canForceMate(const Board& board)
{
	BitBoard nonMinorPcs =
		board.getPieces(PieceType::QUEEN) |
		board.getPieces(PieceType::ROOK) |
		board.getPieces(PieceType::PAWN);

	if (nonMinorPcs != 0)
		return true;

	BitBoard bishops = board.getPieces(PieceType::BISHOP);
	BitBoard knights = board.getPieces(PieceType::KNIGHT);

	BitBoard white = board.getColor(Color::WHITE);
	BitBoard black = board.getColor(Color::BLACK);

	BitBoard blackBishops = black & bishops;
	BitBoard blackKnights = black & knights;
	BitBoard whiteBishops = white & bishops;
	BitBoard whiteKnights = white & knights;

	if ((whiteBishops & LIGHT_SQUARES) && (whiteBishops & DARK_SQUARES))
		return true;

	if ((blackBishops & LIGHT_SQUARES) && (whiteBishops & DARK_SQUARES))
		return true;

	if (whiteBishops && whiteKnights)
		return true;

	if (blackBishops && blackKnights)
		return true;

	if (getPopcnt(whiteKnights) > 2 || getPopcnt(blackKnights) > 2)
		return true;

	return false;
}

}

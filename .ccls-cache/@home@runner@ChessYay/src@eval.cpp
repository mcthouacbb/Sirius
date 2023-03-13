#include "eval.h"

constexpr int QUEEN_VALUE = 900;
constexpr int ROOK_VALUE = 500;
constexpr int BISHOP_VALUE = 300;
constexpr int KNIGHT_VALUE = 300;
constexpr int PAWN_VALUE = 100;

int evaluate(const Board& board)
{
	Color color = board.getCurrPlayer();
	Color oppColor = getOppColor(color);
	int material = 0;

	material += QUEEN_VALUE * (
		static_cast<int>(getPopcnt(board.getPiece(PieceType::QUEEN, color))) -
		static_cast<int>(getPopcnt(board.getPiece(PieceType::QUEEN, oppColor)))
	);
	material += ROOK_VALUE * (
		static_cast<int>(getPopcnt(board.getPiece(PieceType::ROOK, color))) -
		static_cast<int>(getPopcnt(board.getPiece(PieceType::ROOK, oppColor)))
	);
	material += BISHOP_VALUE * (
		static_cast<int>(getPopcnt(board.getPiece(PieceType::BISHOP, color))) -
		static_cast<int>(getPopcnt(board.getPiece(PieceType::BISHOP, oppColor)))
	);
	material += KNIGHT_VALUE * (
		static_cast<int>(getPopcnt(board.getPiece(PieceType::KNIGHT, color))) -
		static_cast<int>(getPopcnt(board.getPiece(PieceType::KNIGHT, oppColor)))
	);
	material += PAWN_VALUE * (
		static_cast<int>(getPopcnt(board.getPiece(PieceType::PAWN, color))) -
		static_cast<int>(getPopcnt(board.getPiece(PieceType::PAWN, oppColor)))
	);
	/*material += QUEEN_VALUE * board.getPiece(PieceType::QUEEN, Color::WHITE);
	material += ROOK_VALUE * board.getPiece(PieceType::ROOK, Color::WHITE);
	material += BISHOP_VALUE * board.getPiece(PieceType::BISHOP, Color::WHITE);
	material += KNIGHT_VALUE * board.getPiece(PieceType::KNIGHT, Color::WHITE);
	material += PAWN_VALUE * board.getPiece(PieceType::PAWN, Color::WHITE);
	
	material -= QUEEN_VALUE * board.getPiece(PieceType::QUEEN, Color::BLACK);
	material -= ROOK_VALUE * board.getPiece(PieceType::ROOK, Color::BLACK);
	material -= BISHOP_VALUE * board.getPiece(PieceType::BISHOP, Color::BLACK);
	material -= KNIGHT_VALUE * board.getPiece(PieceType::KNIGHT, Color::BLACK);
	material -= PAWN_VALUE * board.getPiece(PieceType::PAWN, Color::BLACK);*/
	return material;
	// return eval * (color == Color::WHITE ? 1 : -1);
}
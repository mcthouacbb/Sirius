#pragma once

#include "defs.h"
#include "bitboard.h"
#include "eval/eval_state.h"

#include <string_view>
#include <string>

struct BoardState
{
	int halfMoveClock;
	int epSquare;
	int castlingRights;

	Piece srcPiece;
	Piece dstPiece;
};

class Board
{
public:
	Board();

	Board(const Board&) = delete;
	Board& operator=(const Board&) = delete;

	void setToFen(const std::string_view& fen);

	std::string stringRep() const;
	std::string fenStr() const;

	void printDbg() const;

	void makeMove(Move move, BoardState& state);
	void unmakeMove(Move move, const BoardState& state);

	Color currPlayer() const;
	int epSquare() const;
	int gamePly() const;
	int halfMoveClock() const;
	int castlingRights() const;

	Piece getPieceAt(uint32_t square) const;
	BitBoard getPieces(PieceType type) const;
	BitBoard getColor(Color color) const;
	BitBoard getAllPieces() const;

	const eval::EvalState& evalState() const;
private:
	void addPiece(int pos, Color color, PieceType piece);
	void addPiece(int pos, Piece piece);
	void removePiece(int pos);
	void movePiece(int src, int dst);

	Piece m_Squares[64];
	BitBoard m_Pieces[7];
	BitBoard m_Colors[2];

	Color m_CurrPlayer;

	eval::EvalState m_EvalState;

	int m_GamePly;
	int m_Enpassant;
	int m_HalfMoveClock;
	int m_CastlingRights;
};

inline Color Board::currPlayer() const
{
	return m_CurrPlayer;
}

inline int Board::epSquare() const
{
	return m_Enpassant;
}

inline int Board::gamePly() const
{
	return m_GamePly;
}

inline int Board::halfMoveClock() const
{
	return m_HalfMoveClock;
}

inline int Board::castlingRights() const
{
	return m_CastlingRights;
}

inline Piece Board::getPieceAt(uint32_t square) const
{
	return m_Squares[square];
}

inline BitBoard Board::getPieces(PieceType type) const
{
	return m_Pieces[static_cast<int>(type)];
}

inline BitBoard Board::getColor(Color color) const
{
	return m_Colors[static_cast<int>(color)];
}

inline BitBoard Board::getAllPieces() const
{
	return m_Pieces[0];
}

inline const eval::EvalState& Board::evalState() const
{
	return m_EvalState;
}
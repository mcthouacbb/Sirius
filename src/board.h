#pragma once

#include "defs.h"
#include "bitboard.h"
#include "eval/eval_state.h"
#include "zobrist.h"

#include <string_view>
#include <string>

struct CheckInfo
{
	BitBoard checkers;
	BitBoard pinners[2];
	BitBoard blockers[2];
};

struct BoardState
{
	int halfMoveClock;
	int reversiblePly;
	int epSquare;
	int castlingRights;
	ZKey zkey;
	CheckInfo checkInfo;

	Piece capturedPiece;

	BoardState* prev;
};

class Board
{
public:
	Board();

	Board(const Board&) = delete;
	Board& operator=(const Board&) = delete;

	void setToFen(const std::string_view& fen);

	std::string stringRep() const;

	void printDbg() const;

	void makeMove(Move move, BoardState& state);
	void unmakeMove(Move move);

	Color sideToMove() const;
	int epSquare() const;
	int gamePly() const;
	int halfMoveClock() const;
	int reversiblePly() const;
	int castlingRights() const;
	ZKey zkey() const;

	int repetitions() const;

	Piece getPieceAt(uint32_t square) const;
	BitBoard getPieces(PieceType type) const;
	BitBoard getPieces(Color color, PieceType type) const;
	BitBoard getColor(Color color) const;
	BitBoard getAllPieces() const;

	bool squareAttacked(Color color, uint32_t square) const;
	bool squareAttacked(Color color, uint32_t square, BitBoard blockers) const;
	BitBoard attackersTo(Color color, uint32_t square) const;
	BitBoard attackersTo(Color color, uint32_t square, BitBoard blockers) const;
	BitBoard pinnersBlockers(uint32_t square, BitBoard attackers, BitBoard& pinners) const;

	BitBoard checkers() const;
	BitBoard pinners(Color color) const;
	BitBoard checkBlockers(Color color) const;

	const eval::EvalState& evalState() const;
private:
	void updateCheckInfo();
	void addPiece(int pos, Color color, PieceType piece);
	void addPiece(int pos, Piece piece);
	void removePiece(int pos);
	void movePiece(int src, int dst);

	Piece m_Squares[64];
	BitBoard m_Pieces[7];
	BitBoard m_Colors[2];

	Color m_SideToMove;

	eval::EvalState m_EvalState;

	int m_GamePly;
	int m_HalfMoveClock;
	int m_ReversiblePly;
	int m_EpSquare;
	int m_CastlingRights;

	ZKey m_ZKey;

	CheckInfo m_CheckInfo;

	BoardState* m_State;
};

inline Color Board::sideToMove() const
{
	return m_SideToMove;
}

inline int Board::epSquare() const
{
	return m_EpSquare;
}

inline int Board::gamePly() const
{
	return m_GamePly;	
}

inline int Board::halfMoveClock() const
{
	return m_HalfMoveClock;	
}

inline int Board::reversiblePly() const
{
	return m_ReversiblePly;
}

inline int Board::castlingRights() const
{
	return m_CastlingRights;	
}

inline ZKey Board::zkey() const
{
	return m_ZKey;
}

inline Piece Board::getPieceAt(uint32_t square) const
{
	return m_Squares[square];
}

inline BitBoard Board::getPieces(PieceType type) const
{
	return m_Pieces[static_cast<int>(type)];
}

inline BitBoard Board::getPieces(Color color, PieceType type) const
{
	return getPieces(type) & getColor(color);
}

inline BitBoard Board::getColor(Color color) const
{
	return m_Colors[static_cast<int>(color)];
}

inline BitBoard Board::getAllPieces() const
{
	return m_Pieces[0];
}

inline bool Board::squareAttacked(Color color, uint32_t square) const
{
	return squareAttacked(color, square, getAllPieces());
}

inline BitBoard Board::attackersTo(Color color, uint32_t square) const
{
	return attackersTo(color, square, getAllPieces());
}

inline const eval::EvalState& Board::evalState() const
{
	return m_EvalState;
}

inline BitBoard Board::checkers() const
{
	return m_CheckInfo.checkers;
}

inline BitBoard Board::pinners(Color color) const
{
	return m_CheckInfo.pinners[static_cast<int>(color)];
}

inline BitBoard Board::checkBlockers(Color color) const
{
	return m_CheckInfo.blockers[static_cast<int>(color)];
}
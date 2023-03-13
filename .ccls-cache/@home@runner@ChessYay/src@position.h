#pragma once

#include "bitboard.h"
#include "piece.h"
#include "move.h"

class Position
{
public:
	Position();
	Position(const std::string& fen);

	BitBoard getPiece(Color color, PieceType pieceType) const;
	BitBoard getPieces(Color color) const;
	BitBoard getAllPieces() const;

	void setToFen(const std::string& fen);

	void makeMove(Move move);
	void unmakeMove(Move move);
private:
	BitBoard& whitePawns();
	BitBoard& whiteKing();
	BitBoard& whiteQueens();
	BitBoard& whiteBishops();
	BitBoard& whiteKnights();
	BitBoard& whiteRooks();

	BitBoard& blackPawns();
	BitBoard& blackKing();
	BitBoard& blackQueens();
	BitBoard& blackBishops();
	BitBoard& blackKnights();
	BitBoard& blackRooks();

	BitBoard& whitePieces();
	BitBoard& blackPieces();

	BitBoard& getBitBoard(Color color, PieceType pieceType);

	void clearState();
	void updateBitboards();

	BitBoard m_AllPieces;
	BitBoard m_PieceColors[2];

	BitBoard m_PieceTypes[2][NUM_PIECES];

	/*BitBoard whitePawns;
	BitBoard whiteKing;
	BitBoard whiteQueens;
	BitBoard whiteRooks;
	BitBoard whiteKnights;
	BitBoard whiteBishops;

	BitBoard blackPawns;
	BitBoard blackKing;
	BitBoard blackQueens;
	BitBoard blackRooks;
	BitBoard blackKnights;
	BitBoard blackBishops;*/
};

inline BitBoard Position::getPiece(Color color, PieceType pieceType) const
{
	return m_PieceTypes[static_cast<uint32_t>(color)][static_cast<uint32_t>(pieceType)];
}

inline BitBoard Position::getPieces(Color color) const
{
	return m_PieceColors[static_cast<uint32_t>(color)];
}

inline BitBoard Position::getAllPieces() const
{
	return m_AllPieces;
}
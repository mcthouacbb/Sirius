#pragma once

#include "bitboard.h"
#include "piece.h"
#include "move.h"

struct MoveGenInfo
{
	BitBoard checkers;
};

class Board
{
public:
	Board();
	Board(const std::string& fen);

	BitBoard getPiece(PieceType pieceType, Color color) const;
	BitBoard getColor(Color color) const;
	BitBoard getAllPieces() const;
	BitBoard getEnpassant() const;
	Color getCurrPlayer() const;
	bool getCastlingRights(Color color, CastleSide side) const;
	PieceType getWhitePieceAt(uint32_t idx) const;
	PieceType getWhitePieceFrom(BitBoard bb) const;
	PieceType getBlackPieceAt(uint32_t idx) const;
	PieceType getBlackPieceFrom(BitBoard bb) const;

	MoveGenInfo& moveGenInfo();

	bool isInCheck();

	void setToFen(const std::string& fen);

	void makeMove(Move move);
	void unmakeMove(Move move);

	std::string getString();
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

	const BitBoard& whitePawns() const;
	const BitBoard& whiteKing() const;
	const BitBoard& whiteQueens() const;
	const BitBoard& whiteBishops() const;
	const BitBoard& whiteKnights() const;
	const BitBoard& whiteRooks() const;

	const BitBoard& blackPawns() const;
	const BitBoard& blackKing() const;
	const BitBoard& blackQueens() const;
	const BitBoard& blackBishops() const;
	const BitBoard& blackKnights() const;
	const BitBoard& blackRooks() const;

	const BitBoard& whitePieces() const;
	const BitBoard& blackPieces() const;

	BitBoard& getBitBoard(PieceType pieceType, Color color);
	BitBoard& getBitBoard(Color color);

	void clearState();
	void updateBitboards();

	void pushState();
	void popState();

	BitBoard m_AllPieces;
	BitBoard m_PieceColors[2];

	BitBoard m_PieceTypes[2][NUM_PIECES];

	BitBoard m_Enpassant;

	Color m_CurrPlayer;

	uint32_t m_CastlingRights;

	uint32_t m_GamePly;

	MoveGenInfo m_MoveGenInfo = {};

	uint32_t* m_PrevState = m_PrevStates;
	uint32_t m_PrevStates[256];
	//std::deque<uint32_t> m_PrevStates;
};

inline BitBoard Board::getPiece(PieceType pieceType, Color color) const
{
	return m_PieceTypes[static_cast<uint32_t>(color)][static_cast<uint32_t>(pieceType)];
}

inline BitBoard Board::getColor(Color color) const
{
	return m_PieceColors[static_cast<uint32_t>(color)];
}

inline BitBoard Board::getAllPieces() const
{
	return m_AllPieces;
}

inline BitBoard Board::getEnpassant() const
{
	return m_Enpassant;
}

inline Color Board::getCurrPlayer() const
{
	return m_CurrPlayer;
}

inline bool Board::getCastlingRights(Color color, CastleSide side) const
{
	return (m_CastlingRights >> (static_cast<uint32_t>(color) * 2 + static_cast<uint32_t>(side))) & 1;
}

inline MoveGenInfo& Board::moveGenInfo()
{
	return m_MoveGenInfo;
}

inline bool Board::isInCheck()
{
	return m_MoveGenInfo.checkers != 0;
}
#include "board.h"

Board::Board()
{
	whitePieces() = ranks[0] | ranks[1];
	// whitePawns() = ranks[1];
	// whiteKing() = ONE << 4;
	// whiteQueens() = ONE << 3;
	// whiteRooks() = ONE | ONE << 7;
	// whiteKnights() = ONE << 1 | ONE << 6;
	// whiteBishops() = ONE << 2 | ONE << 5;

	whitePawns() = ranks[1];
	whiteKing() = ONE << 4;
	whiteQueens() = ONE << 3;
	whiteRooks() = ONE | ONE << 7;
	whiteKnights() = ONE << 1 | ONE << 6;
	whiteBishops() = ONE << 2 | ONE << 5;

	blackPieces() = ranks[6] | ranks[7];
	// blackPawns() = ranks[6];
	// blackKing() = whiteKing() << 56;
	// blackQueens() = whiteQueens() << 56;
	// blackRooks() = whiteRooks() << 56;
	// blackKnights() = whiteKnights() << 56;
	// blackBishops() = whiteBishops() << 56;

	blackPawns() = ranks[6];
	blackKing() = whiteKing() << 56;
	blackQueens() = whiteQueens() << 56;
	blackRooks() = whiteRooks() << 56;
	blackKnights() = whiteKnights() << 56;
	blackBishops() = whiteBishops() << 56;

	m_AllPieces = whitePieces() | blackPieces();

	m_Enpassant = 0;
	m_CurrPlayer = Color::WHITE;
	m_CastlingRights = 0xF;
}

Board::Board(const std::string& fen)
{
	setToFen(fen);
}
PieceType Board::getWhitePieceAt(uint32_t idx) const
{
	return getWhitePieceFrom(1ull << idx);
}

PieceType Board::getWhitePieceFrom(BitBoard bb) const
{
	if ((whitePieces() & bb) == 0)
		return PieceType::NONE;
	if (whitePawns() & bb)
		return PieceType::PAWN;
	if (whiteQueens() & bb)
		return PieceType::QUEEN;
	if (whiteRooks() & bb)
		return PieceType::ROOK;
	if (whiteBishops() & bb)
		return PieceType::BISHOP;
	if (whiteKnights() & bb)
		return PieceType::KNIGHT;
	if (whiteKing() & bb)
		return PieceType::KING;
	throw "white piece error";
}

PieceType Board::getBlackPieceAt(uint32_t idx) const
{
	return getBlackPieceFrom(1ull << idx);
}

PieceType Board::getBlackPieceFrom(BitBoard bb) const
{
	if ((blackPieces() & bb) == 0)
		return PieceType::NONE;
	if (blackPawns() & bb)
		return PieceType::PAWN;
	if (blackQueens() & bb)
		return PieceType::QUEEN;
	if (blackRooks() & bb)
		return PieceType::ROOK;
	if (blackBishops() & bb)
		return PieceType::BISHOP;
	if (blackKnights() & bb)
		return PieceType::KNIGHT;
	if (blackKing() & bb)
		return PieceType::KING;
	throw "black piece error";
}


void Board::setToFen(const std::string& fen)
{
	clearState();

	size_t i;

	uint32_t currPos = 56;

	for (i = 0; i < fen.size(); i++)
	{
		switch (fen[i])
		{
			case ' ':
				i++;
				goto done;
			case '/':
				// std::cout << currPos << std::endl;
				currPos -= 16;
				continue;
			case 'p':
				blackPawns() |= ONE << currPos;
				break;
			case 'n':
				blackKnights() |= ONE << currPos;
				break;
			case 'b':
				blackBishops() |= ONE << currPos;
				break;
			case 'r':
				blackRooks() |= ONE << currPos;
				break;
			case 'q':
				blackQueens() |= ONE << currPos;
				break;
			case 'k':
				blackKing() |= ONE << currPos;
				break;
			case 'P':
				whitePawns() |= ONE << currPos;
				break;
			case 'N':
				whiteKnights() |= ONE << currPos;
				break;
			case 'B':
				whiteBishops() |= ONE << currPos;
				break;
			case 'R':
				whiteRooks() |= ONE << currPos;
				break;
			case 'Q':
				whiteQueens() |= ONE << currPos;
				break;
			case 'K':
				whiteKing() |= ONE << currPos;
				break;
			default:
				currPos += fen[i] - '0';
				continue;
		}
		currPos++;
	}
done:
	m_CurrPlayer = fen[i] == 'w' ? Color::WHITE : Color::BLACK;
	i += 2;
	if (fen[i] == '-')
		m_CastlingRights = 0;
	else
	{
		if (fen[i] == 'K')
		{
			m_CastlingRights |= 1;
			i++;
		}
		if (fen[i] == 'Q')
		{
			m_CastlingRights |= 2;
			i++;
		}
		if (fen[i] == 'k')
		{
			m_CastlingRights |= 4;
			i++;
		}
		if (fen[i] == 'q')
		{
			m_CastlingRights |= 8;
			i++;
		}
	}
	// TODO: EXTRA FEN STUFF
	updateBitboards();
	return;
}

void Board::makeMove(Move move)
{
	pushState();

	Color color = m_CurrPlayer;
	Color oppColor = m_CurrPlayer = getOppColor(m_CurrPlayer);
	switch (move.type())
	{
		case MoveType::NONE:
		{
			BitBoard moveBB = (1ull << move.srcPos()) | (1ull << move.dstPos());
			getBitBoard(move.srcPieceType(), color) ^= moveBB;
			getBitBoard(color) ^= moveBB;
			m_AllPieces ^= moveBB;
			break;
		}
		case MoveType::CAPTURE:
		{
			BitBoard srcBB = 1ull << move.srcPos();
			BitBoard captureBB = 1ull << move.dstPos();
			BitBoard moveBB = srcBB | captureBB;
			getBitBoard(move.srcPieceType(), color) ^= moveBB;
			getBitBoard(move.dstPieceType(), oppColor) ^= captureBB;
			getBitBoard(color) ^= moveBB;
			getBitBoard(oppColor) ^= captureBB;
			m_AllPieces ^= srcBB;
			break;
		}
		case MoveType::DOUBLE_PUSH:
		{
			m_Enpassant = 1ull << (move.srcPos() + (color == Color::WHITE ? 8 : -8));
			BitBoard moveBB = (1ull << move.srcPos()) | (1ull << move.dstPos());
			getBitBoard(move.srcPieceType(), color) ^= moveBB;
			getBitBoard(color) ^= moveBB;
			m_AllPieces ^= moveBB;
			break;
		}
		case MoveType::PROMOTION:
		{
			BitBoard pawnBB = 1ull << move.srcPos();
			getBitBoard(PieceType::PAWN, color) ^= pawnBB;

			BitBoard promotionBB = 1ull << move.dstPos();
			getBitBoard(move.dstPieceType(), color) ^= promotionBB;

			BitBoard moveBB = promotionBB | pawnBB;
			getBitBoard(color) ^= moveBB;
			m_AllPieces ^= moveBB;
			break;
		}
		case MoveType::CAPTURE_PROMOTION:
		{
			BitBoard pawnBB = 1ull << move.srcPos();
			getBitBoard(PieceType::PAWN, color) ^= pawnBB;

			BitBoard capturePromotionBB = 1ull << move.dstPos();
			getBitBoard(move.srcPieceType(), oppColor) ^= capturePromotionBB;
			getBitBoard(move.dstPieceType(), color) ^= capturePromotionBB;

			BitBoard moveBB = pawnBB | capturePromotionBB;
			getBitBoard(color) ^= moveBB;
			getBitBoard(oppColor) ^= capturePromotionBB;
			m_AllPieces ^= pawnBB;
			break;
		}
		case MoveType::ENPASSANT:
		{
			BitBoard moveBB = (1ull << move.srcPos()) | (1ull << move.dstPos());
			getBitBoard(PieceType::PAWN, color) ^= moveBB;
			getBitBoard(color) ^= moveBB;

			BitBoard enpassantBB = color == Color::WHITE ? 1ull << (move.dstPos() - 8) : 1ull << (move.dstPos() + 8);
			getBitBoard(PieceType::PAWN, oppColor) ^= enpassantBB;
			getBitBoard(oppColor) ^= enpassantBB;
			m_AllPieces ^= moveBB | enpassantBB;
			break;
		}
		case MoveType::KSIDE_CASTLE:
		{
			BitBoard kingBB = (1ull << move.srcPos()) | (1ull << move.dstPos());
			BitBoard rookBB = kingBB << 1;
			getBitBoard(PieceType::KING, color) ^= kingBB;
			getBitBoard(PieceType::ROOK, color) ^= rookBB;
			BitBoard moveBB = kingBB | rookBB;
			getBitBoard(color) ^= moveBB;
			m_AllPieces ^= moveBB;
			break;
		}
		case MoveType::QSIDE_CASTLE:
		{
			BitBoard kingBB = (1ull << move.srcPos()) | (1ull << move.dstPos());
			BitBoard rookBB = (1ull << (move.srcPos() - 1)) | (1ull << (move.dstPos() - 2));
			getBitBoard(PieceType::KING, color) ^= kingBB;
			getBitBoard(PieceType::ROOK, color) ^= rookBB;
			BitBoard moveBB = kingBB | rookBB;
			getBitBoard(color) ^= moveBB;
			m_AllPieces ^= moveBB;
			break;
		}
	}

	if (move.type() != MoveType::DOUBLE_PUSH)
		m_Enpassant = 0;

	if (move.srcPos() == (color == Color::WHITE ? 4 : 60))
		m_CastlingRights &= color == Color::WHITE ? ~0x3 : ~0xC;

	uint32_t ksRookSquare = color == Color::WHITE ? 7 : 63;
	if (move.srcPos() == ksRookSquare)
	{
		m_CastlingRights &= color == Color::WHITE ? ~0x1 : ~0x4;
	}

	uint32_t ksRookSquareOpp = oppColor == Color::WHITE ? 7 : 63;
	if (move.dstPos() == ksRookSquareOpp)
	{
		m_CastlingRights &= oppColor == Color::WHITE ? ~0x1 : ~0x4;
	}

	uint32_t qsRookSquare = color == Color::WHITE ? 0 : 56;
	if (move.srcPos() == qsRookSquare)
	{
		m_CastlingRights &= color == Color::WHITE ? ~0x2 : ~0x8;
	}

	uint32_t qsRookSquareOpp = oppColor == Color::WHITE ? 0 : 56;
	if (move.dstPos() == qsRookSquareOpp)
	{
		m_CastlingRights &= oppColor == Color::WHITE ? ~0x2 : ~0x8;
	}
	/*if (whitePieces() != (whitePawns() | whiteKnights() | whiteKing() | whiteQueens() | whiteRooks() | whiteBishops()))
		__debugbreak();

	if (blackPieces() != (blackPawns() | blackKnights() | blackKing() | blackQueens() | blackRooks() | blackBishops()))
		__debugbreak();

	if (m_AllPieces != (whitePieces() | blackPieces()))
		__debugbreak();*/
}

void Board::unmakeMove(Move move)
{
	popState();

	Color oppColor = m_CurrPlayer;
	Color color = m_CurrPlayer = getOppColor(m_CurrPlayer);
	switch (move.type())
	{
		case MoveType::NONE:
		{
			BitBoard moveBB = (1ull << move.srcPos()) | (1ull << move.dstPos());
			getBitBoard(move.srcPieceType(), color) ^= moveBB;
			getBitBoard(color) ^= moveBB;
			m_AllPieces ^= moveBB;
			break;
		}
		case MoveType::CAPTURE:
		{
			BitBoard srcBB = 1ull << move.srcPos();
			BitBoard captureBB = 1ull << move.dstPos();
			BitBoard moveBB = srcBB | captureBB;
			getBitBoard(move.srcPieceType(), color) ^= moveBB;
			getBitBoard(move.dstPieceType(), oppColor) ^= captureBB;
			getBitBoard(color) ^= moveBB;
			getBitBoard(oppColor) ^= captureBB;
			m_AllPieces ^= srcBB;
			break;
		}
		case MoveType::DOUBLE_PUSH:
		{
			BitBoard moveBB = (1ull << move.srcPos()) | (1ull << move.dstPos());
			getBitBoard(move.srcPieceType(), color) ^= moveBB;
			getBitBoard(color) ^= moveBB;
			m_AllPieces ^= moveBB;
			break;
		}
		case MoveType::PROMOTION:
		{
			BitBoard pawnBB = 1ull << move.srcPos();
			getBitBoard(PieceType::PAWN, color) ^= pawnBB;

			BitBoard promotionBB = 1ull << move.dstPos();
			getBitBoard(move.dstPieceType(), color) ^= promotionBB;

			BitBoard moveBB = promotionBB | pawnBB;
			getBitBoard(color) ^= moveBB;
			m_AllPieces ^= moveBB;
			break;
		}
		case MoveType::CAPTURE_PROMOTION:
		{
			BitBoard pawnBB = 1ull << move.srcPos();
			getBitBoard(PieceType::PAWN, color) ^= pawnBB;

			BitBoard capturePromotionBB = 1ull << move.dstPos();
			getBitBoard(move.srcPieceType(), oppColor) ^= capturePromotionBB;
			getBitBoard(move.dstPieceType(), color) ^= capturePromotionBB;

			BitBoard moveBB = pawnBB | capturePromotionBB;
			getBitBoard(color) ^= moveBB;
			getBitBoard(oppColor) ^= capturePromotionBB;
			m_AllPieces ^= pawnBB;
			break;
		}
		case MoveType::ENPASSANT:
		{
			BitBoard moveBB = (1ull << move.srcPos()) | (1ull << move.dstPos());
			getBitBoard(PieceType::PAWN, color) ^= moveBB;
			getBitBoard(color) ^= moveBB;

			BitBoard enpassantBB = color == Color::WHITE ? 1ull << (move.dstPos() - 8) : 1ull << (move.dstPos() + 8);
			getBitBoard(PieceType::PAWN, oppColor) ^= enpassantBB;
			getBitBoard(oppColor) ^= enpassantBB;
			m_AllPieces ^= moveBB | enpassantBB;
			break;
		}
		case MoveType::KSIDE_CASTLE:
		{
			BitBoard kingBB = (1ull << move.srcPos()) | (1ull << move.dstPos());
			BitBoard rookBB = kingBB << 1;
			getBitBoard(PieceType::KING, color) ^= kingBB;
			getBitBoard(PieceType::ROOK, color) ^= rookBB;
			BitBoard moveBB = kingBB | rookBB;
			getBitBoard(color) ^= moveBB;
			m_AllPieces ^= moveBB;
			break;
		}
		case MoveType::QSIDE_CASTLE:
		{
			BitBoard kingBB = (1ull << move.srcPos()) | (1ull << move.dstPos());
			BitBoard rookBB = (1ull << (move.srcPos() - 1)) | (1ull << (move.dstPos() - 2));
			getBitBoard(PieceType::KING, color) ^= kingBB;
			getBitBoard(PieceType::ROOK, color) ^= rookBB;
			BitBoard moveBB = kingBB | rookBB;
			getBitBoard(color) ^= moveBB;
			m_AllPieces ^= moveBB;
			break;
		}
	}

	/*if (whitePieces() != (whitePawns() | whiteKnights() | whiteKing() | whiteQueens() | whiteRooks() | whiteBishops()))
		__debugbreak();

	if (blackPieces() != (blackPawns() | blackKnights() | blackKing() | blackQueens() | blackRooks() | blackBishops()))
		__debugbreak();

	if (m_AllPieces != (whitePieces() | blackPieces()))
		__debugbreak();*/
}

char blackPieceChars[6] = {
	'p', 'k', 'q', 'r', 'b', 'n'
};
char whitePiecesChars[6] = {
	'P', 'K', 'Q', 'R', 'B', 'N'
};

std::string Board::getString()
{
	const char* between = "+---+---+---+---+---+---+---+---+\n";
	std::string result;
	for (int j = 56; j >= 0; j -= 8)
	{
		result += between;

		for (int i = j; i < j + 8; i++)
		{
			result += "| ";
			PieceType piece = getWhitePieceAt(i);
			if (piece == PieceType::NONE)
			{
				piece = getBlackPieceAt(i);
				if (piece == PieceType::NONE)
				{
					result += ' ';
				}
				else
				{
					result += blackPieceChars[(int)piece];
				}
			}
			else
			{
				result += whitePiecesChars[(int)piece];
			}

			result += " ";
		}
		result += "|\n";
	}
	result += between;
	return result;
}

void Board::clearState()
{
	whitePawns() = 0;
	whiteKing() = 0;
	whiteQueens() = 0;
	whiteRooks() = 0;
	whiteKnights() = 0;
	whiteBishops() = 0;
	blackPawns() = 0;
	blackKing() = 0;
	blackQueens() = 0;
	blackRooks() = 0;
	blackKnights() = 0;
	blackBishops() = 0;

	m_AllPieces = 0;
	whitePieces() = 0;
	blackPieces() = 0;

	m_PrevState = m_PrevStates;
	m_CastlingRights = 0;
	m_CurrPlayer = Color::WHITE;
	m_Enpassant = 0;
}

void Board::updateBitboards()
{
	whitePieces() =
		whitePawns() |
		whiteKing() |
		whiteQueens() |
		whiteRooks() |
		whiteBishops() |
		whiteKnights();
	blackPieces() =
		blackPawns() |
		blackKing() |
		blackQueens() |
		blackRooks() |
		blackBishops() |
		blackKnights();
	m_AllPieces = whitePieces() | blackPieces();
}

void Board::pushState()
{
	uint32_t epFile;
	if (m_Enpassant == 0)
	{
		epFile = 8;
	}
	else
	{
		epFile = getLSB(m_Enpassant) & 7;
	}
	*m_PrevState++ = m_CastlingRights | (epFile << 4);
	//m_PrevStates.push_back(m_CastlingRights | epFile << 4);
}

void Board::popState()
{
	uint32_t state = *--m_PrevState;
	//m_PrevStates.pop_back();
	m_CastlingRights = state & 0xF;
	uint32_t epFile = state >> 4;

	if (epFile != 8)
	{
		m_Enpassant = (1ull << epFile) << (m_CurrPlayer == Color::WHITE ? 16 : 40);
	}
	else
	{
		m_Enpassant = 0;
	}
}

BitBoard& Board::whitePieces()
{
	return m_PieceColors[static_cast<uint32_t>(Color::WHITE)];
}

BitBoard& Board::blackPieces()
{
	return m_PieceColors[static_cast<uint32_t>(Color::BLACK)];
}

BitBoard& Board::whitePawns()
{
	return m_PieceTypes[static_cast<uint32_t>(Color::WHITE)][static_cast<uint32_t>(PieceType::PAWN)];
}

BitBoard& Board::whiteKing()
{
	return m_PieceTypes[static_cast<uint32_t>(Color::WHITE)][static_cast<uint32_t>(PieceType::KING)];
}

BitBoard& Board::whiteQueens()
{
	return m_PieceTypes[static_cast<uint32_t>(Color::WHITE)][static_cast<uint32_t>(PieceType::QUEEN)];
}

BitBoard& Board::whiteBishops()
{
	return m_PieceTypes[static_cast<uint32_t>(Color::WHITE)][static_cast<uint32_t>(PieceType::BISHOP)];
}

BitBoard& Board::whiteKnights()
{
	return m_PieceTypes[static_cast<uint32_t>(Color::WHITE)][static_cast<uint32_t>(PieceType::KNIGHT)];
}

BitBoard& Board::whiteRooks()
{
	return m_PieceTypes[static_cast<uint32_t>(Color::WHITE)][static_cast<uint32_t>(PieceType::ROOK)];
}


BitBoard& Board::blackPawns()
{
	return m_PieceTypes[static_cast<uint32_t>(Color::BLACK)][static_cast<uint32_t>(PieceType::PAWN)];
}

BitBoard& Board::blackKing()
{
	return m_PieceTypes[static_cast<uint32_t>(Color::BLACK)][static_cast<uint32_t>(PieceType::KING)];
}

BitBoard& Board::blackQueens()
{
	return m_PieceTypes[static_cast<uint32_t>(Color::BLACK)][static_cast<uint32_t>(PieceType::QUEEN)];
}

BitBoard& Board::blackBishops()
{
	return m_PieceTypes[static_cast<uint32_t>(Color::BLACK)][static_cast<uint32_t>(PieceType::BISHOP)];
}

BitBoard& Board::blackKnights()
{
	return m_PieceTypes[static_cast<uint32_t>(Color::BLACK)][static_cast<uint32_t>(PieceType::KNIGHT)];
}

BitBoard& Board::blackRooks()
{
	return m_PieceTypes[static_cast<uint32_t>(Color::BLACK)][static_cast<uint32_t>(PieceType::ROOK)];
}

BitBoard& Board::getBitBoard(PieceType pieceType, Color color)
{
	return m_PieceTypes[static_cast<uint32_t>(color)][static_cast<uint32_t>(pieceType)];
}

BitBoard& Board::getBitBoard(Color color)
{
	return m_PieceColors[static_cast<uint32_t>(color)];
}

const BitBoard& Board::whitePieces() const
{
	return m_PieceColors[static_cast<uint32_t>(Color::WHITE)];
}

const BitBoard& Board::blackPieces() const
{
	return m_PieceColors[static_cast<uint32_t>(Color::BLACK)];
}

const BitBoard& Board::whitePawns() const
{
	return m_PieceTypes[static_cast<uint32_t>(Color::WHITE)][static_cast<uint32_t>(PieceType::PAWN)];
}

const BitBoard& Board::whiteKing() const
{
	return m_PieceTypes[static_cast<uint32_t>(Color::WHITE)][static_cast<uint32_t>(PieceType::KING)];
}

const BitBoard& Board::whiteQueens() const
{
	return m_PieceTypes[static_cast<uint32_t>(Color::WHITE)][static_cast<uint32_t>(PieceType::QUEEN)];
}

const BitBoard& Board::whiteBishops() const
{
	return m_PieceTypes[static_cast<uint32_t>(Color::WHITE)][static_cast<uint32_t>(PieceType::BISHOP)];
}

const BitBoard& Board::whiteKnights() const
{
	return m_PieceTypes[static_cast<uint32_t>(Color::WHITE)][static_cast<uint32_t>(PieceType::KNIGHT)];
}

const BitBoard& Board::whiteRooks() const
{
	return m_PieceTypes[static_cast<uint32_t>(Color::WHITE)][static_cast<uint32_t>(PieceType::ROOK)];
}


const BitBoard& Board::blackPawns() const
{
	return m_PieceTypes[static_cast<uint32_t>(Color::BLACK)][static_cast<uint32_t>(PieceType::PAWN)];
}

const BitBoard& Board::blackKing() const
{
	return m_PieceTypes[static_cast<uint32_t>(Color::BLACK)][static_cast<uint32_t>(PieceType::KING)];
}

const BitBoard& Board::blackQueens() const
{
	return m_PieceTypes[static_cast<uint32_t>(Color::BLACK)][static_cast<uint32_t>(PieceType::QUEEN)];
}

const BitBoard& Board::blackBishops() const
{
	return m_PieceTypes[static_cast<uint32_t>(Color::BLACK)][static_cast<uint32_t>(PieceType::BISHOP)];
}

const BitBoard& Board::blackKnights() const
{
	return m_PieceTypes[static_cast<uint32_t>(Color::BLACK)][static_cast<uint32_t>(PieceType::KNIGHT)];
}

const BitBoard& Board::blackRooks() const
{
	return m_PieceTypes[static_cast<uint32_t>(Color::BLACK)][static_cast<uint32_t>(PieceType::ROOK)];
}
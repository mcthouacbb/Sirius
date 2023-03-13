#include "position.h"

Position::Position()
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
}

Position::Position(const std::string& fen)
{
	setToFen(fen);
}

void Position::setToFen(const std::string& fen)
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
	// TODO: EXTRA FEN STUFF
	updateBitboards();
	return;
}

void Position::makeMove(Move move)
{
	switch (move.type())
	{
		case MoveType::NONE:
		{
			auto& bb = getBitBoard(Color::WHITE, move.srcPieceType());
			bb ^= (1 << move.srcPos()) | (1 << move.dstPos());
			break;
		}
		default:
			throw std::runtime_error("Invalid/unsupported move type");
	}
}



void Position::clearState()
{
	whitePawns() = 0;
	whiteKing() = 0;
	whiteQueens() = 0;
	whiteRooks() = 0;
	whiteKnights() = 0;
	whiteBishops() = 0;
	blackPawns() = 0;
	blackBishops() = 0;
	blackQueens() = 0;
	blackRooks() = 0;
	blackKnights() = 0;
	blackBishops() = 0;

	m_AllPieces = 0;
	whitePieces() = 0;
	blackPieces() = 0;
}

void Position::updateBitboards()
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

BitBoard& Position::whitePieces()
{
	return m_PieceColors[static_cast<uint32_t>(Color::WHITE)];
}

BitBoard& Position::blackPieces()
{
	return m_PieceColors[static_cast<uint32_t>(Color::BLACK)];
}

BitBoard& Position::whitePawns()
{
	return m_PieceTypes[static_cast<uint32_t>(Color::WHITE)][static_cast<uint32_t>(PieceType::PAWN)];
}

BitBoard& Position::whiteKing()
{
	return m_PieceTypes[static_cast<uint32_t>(Color::WHITE)][static_cast<uint32_t>(PieceType::KING)];
}

BitBoard& Position::whiteQueens()
{
	return m_PieceTypes[static_cast<uint32_t>(Color::WHITE)][static_cast<uint32_t>(PieceType::QUEEN)];
}

BitBoard& Position::whiteBishops()
{
	return m_PieceTypes[static_cast<uint32_t>(Color::WHITE)][static_cast<uint32_t>(PieceType::BISHOP)];
}

BitBoard& Position::whiteKnights()
{
	return m_PieceTypes[static_cast<uint32_t>(Color::WHITE)][static_cast<uint32_t>(PieceType::KNIGHT)];
}

BitBoard& Position::whiteRooks()
{
	return m_PieceTypes[static_cast<uint32_t>(Color::WHITE)][static_cast<uint32_t>(PieceType::ROOK)];
}


BitBoard& Position::blackPawns()
{
	return m_PieceTypes[static_cast<uint32_t>(Color::BLACK)][static_cast<uint32_t>(PieceType::PAWN)];
}

BitBoard& Position::blackKing()
{
	return m_PieceTypes[static_cast<uint32_t>(Color::BLACK)][static_cast<uint32_t>(PieceType::KING)];
}

BitBoard& Position::blackQueens()
{
	return m_PieceTypes[static_cast<uint32_t>(Color::BLACK)][static_cast<uint32_t>(PieceType::QUEEN)];
}

BitBoard& Position::blackBishops()
{
	return m_PieceTypes[static_cast<uint32_t>(Color::BLACK)][static_cast<uint32_t>(PieceType::BISHOP)];
}

BitBoard& Position::blackKnights()
{
	return m_PieceTypes[static_cast<uint32_t>(Color::BLACK)][static_cast<uint32_t>(PieceType::KNIGHT)];
}

BitBoard& Position::blackRooks()
{
	return m_PieceTypes[static_cast<uint32_t>(Color::BLACK)][static_cast<uint32_t>(PieceType::ROOK)];
}

BitBoard& Position::getBitBoard(Color color, PieceType pieceType)
{
	return m_PieceTypes[static_cast<uint32_t>(color)][static_cast<uint32_t>(pieceType)];
}
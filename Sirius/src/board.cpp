#include "board.h"
#include "attacks.h"

#include <cstring>
#include <charconv>

Board::Board()
	: m_Squares(), m_Pieces(), m_Colors(), m_SideToMove(), m_EvalState(), m_GamePly(), m_State(nullptr), m_RootState(nullptr)
{

}

Board::Board(BoardState& rootState)
	: m_RootState(&rootState)
{
	setToFen(defaultFen);
}

void Board::setToFen(const std::string_view& fen)
{
	m_State = m_RootState;
	m_State->pliesFromNull = 0;
	m_State->repetitions = 0;
	m_State->lastRepetition = 0;


	memset(m_Squares, 0, 64 + 72);
	m_EvalState.init();
	m_State->zkey.value = 0;
	m_State->pawnKey.value = 0;
	int i = 0;
	int sq = 56;
	for (;; i++)
	{
		switch (fen[i])
		{
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
				sq += fen[i] - '0';
				break;
			case 'k':
				m_State->zkey.addPiece(PieceType::KING, Color::BLACK, sq);
				addPiece(sq++, Color::BLACK, PieceType::KING);
				break;
			case 'q':
				m_State->zkey.addPiece(PieceType::QUEEN, Color::BLACK, sq);
				addPiece(sq++, Color::BLACK, PieceType::QUEEN);
				break;
			case 'r':
				m_State->zkey.addPiece(PieceType::ROOK, Color::BLACK, sq);
				addPiece(sq++, Color::BLACK, PieceType::ROOK);
				break;
			case 'b':
				m_State->zkey.addPiece(PieceType::BISHOP, Color::BLACK, sq);
				addPiece(sq++, Color::BLACK, PieceType::BISHOP);
				break;
			case 'n':
				m_State->zkey.addPiece(PieceType::KNIGHT, Color::BLACK, sq);
				addPiece(sq++, Color::BLACK, PieceType::KNIGHT);
				break;
			case 'p':
				m_State->zkey.addPiece(PieceType::PAWN, Color::BLACK, sq);
				m_State->pawnKey.addPiece(PieceType::PAWN, Color::BLACK, sq);
				addPiece(sq++, Color::BLACK, PieceType::PAWN);
				break;
			case 'K':
				m_State->zkey.addPiece(PieceType::KING, Color::WHITE, sq);
				addPiece(sq++, Color::WHITE, PieceType::KING);
				break;
			case 'Q':
				m_State->zkey.addPiece(PieceType::QUEEN, Color::WHITE, sq);
				addPiece(sq++, Color::WHITE, PieceType::QUEEN);
				break;
			case 'R':
				m_State->zkey.addPiece(PieceType::ROOK, Color::WHITE, sq);
				addPiece(sq++, Color::WHITE, PieceType::ROOK);
				break;
			case 'B':
				m_State->zkey.addPiece(PieceType::BISHOP, Color::WHITE, sq);
				addPiece(sq++, Color::WHITE, PieceType::BISHOP);
				break;
			case 'N':
				m_State->zkey.addPiece(PieceType::KNIGHT, Color::WHITE, sq);
				addPiece(sq++, Color::WHITE, PieceType::KNIGHT);
				break;
			case 'P':
				m_State->zkey.addPiece(PieceType::PAWN, Color::WHITE, sq);
				m_State->pawnKey.addPiece(PieceType::PAWN, Color::WHITE, sq);
				addPiece(sq++, Color::WHITE, PieceType::PAWN);
				break;
			case '/':
				sq -= 16;
				break;
			default:
				goto done;
		}
	}
done:
	i++;
	m_SideToMove = fen[i] == 'w' ? Color::WHITE : Color::BLACK;
	if (m_SideToMove == Color::BLACK)
	{
		m_State->zkey.flipSideToMove();
	}

	i += 2;
	m_State->castlingRights = 0;
	if (fen[i] == '-')
	{
		i++;
	}
	else
	{
		if (fen[i] == 'K')
		{
			i++;
			m_State->castlingRights |= 1;
		}

		if (fen[i] == 'Q')
		{
			i++;
			m_State->castlingRights |= 2;
		}

		if (fen[i] == 'k')
		{
			i++;
			m_State->castlingRights |= 4;
		}

		if (fen[i] == 'q')
		{
			i++;
			m_State->castlingRights |= 8;
		}
	}

	m_State->zkey.updateCastlingRights(m_State->castlingRights);

	i++;

	if (fen[i] != '-')
	{
		m_State->epSquare = fen[i] - 'a';
		m_State->epSquare |= (fen[++i] - '1') << 3;
		m_State->zkey.updateEP(m_State->epSquare & 7);
	}
	else
	{
		m_State->epSquare = -1;
	}
	i += 2;

	auto [ptr, ec] = std::from_chars(&fen[i], fen.data() + fen.size(), m_State->halfMoveClock);
	std::from_chars(ptr + 1, fen.data() + fen.size(), m_GamePly);
	m_GamePly = 2 * m_GamePly - 1 - (m_SideToMove == Color::WHITE);

	updateCheckInfo();
}

void Board::setToEpd(const std::string_view& epd)
{
	m_State = m_RootState;
	m_State->pliesFromNull = 0;
	m_State->repetitions = 0;
	m_State->lastRepetition = 0;


	memset(m_Squares, 0, 64 + 72);
	m_EvalState.init();
	m_State->zkey.value = 0;
	m_State->pawnKey.value = 0;
	int i = 0;
	int sq = 56;
	for (;; i++)
	{
		switch (epd[i])
		{
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
				sq += epd[i] - '0';
				break;
			case 'k':
				m_State->zkey.addPiece(PieceType::KING, Color::BLACK, sq);
				addPiece(sq++, Color::BLACK, PieceType::KING);
				break;
			case 'q':
				m_State->zkey.addPiece(PieceType::QUEEN, Color::BLACK, sq);
				addPiece(sq++, Color::BLACK, PieceType::QUEEN);
				break;
			case 'r':
				m_State->zkey.addPiece(PieceType::ROOK, Color::BLACK, sq);
				addPiece(sq++, Color::BLACK, PieceType::ROOK);
				break;
			case 'b':
				m_State->zkey.addPiece(PieceType::BISHOP, Color::BLACK, sq);
				addPiece(sq++, Color::BLACK, PieceType::BISHOP);
				break;
			case 'n':
				m_State->zkey.addPiece(PieceType::KNIGHT, Color::BLACK, sq);
				addPiece(sq++, Color::BLACK, PieceType::KNIGHT);
				break;
			case 'p':
				m_State->zkey.addPiece(PieceType::PAWN, Color::BLACK, sq);
				m_State->pawnKey.addPiece(PieceType::PAWN, Color::BLACK, sq);
				addPiece(sq++, Color::BLACK, PieceType::PAWN);
				break;
			case 'K':
				m_State->zkey.addPiece(PieceType::KING, Color::WHITE, sq);
				addPiece(sq++, Color::WHITE, PieceType::KING);
				break;
			case 'Q':
				m_State->zkey.addPiece(PieceType::QUEEN, Color::WHITE, sq);
				addPiece(sq++, Color::WHITE, PieceType::QUEEN);
				break;
			case 'R':
				m_State->zkey.addPiece(PieceType::ROOK, Color::WHITE, sq);
				addPiece(sq++, Color::WHITE, PieceType::ROOK);
				break;
			case 'B':
				m_State->zkey.addPiece(PieceType::BISHOP, Color::WHITE, sq);
				addPiece(sq++, Color::WHITE, PieceType::BISHOP);
				break;
			case 'N':
				m_State->zkey.addPiece(PieceType::KNIGHT, Color::WHITE, sq);
				addPiece(sq++, Color::WHITE, PieceType::KNIGHT);
				break;
			case 'P':
				m_State->zkey.addPiece(PieceType::PAWN, Color::WHITE, sq);
				m_State->pawnKey.addPiece(PieceType::PAWN, Color::WHITE, sq);
				addPiece(sq++, Color::WHITE, PieceType::PAWN);
				break;
			case '/':
				sq -= 16;
				break;
			default:
				goto done;
		}
	}
done:
	i++;
	m_SideToMove = epd[i] == 'w' ? Color::WHITE : Color::BLACK;
	if (m_SideToMove == Color::BLACK)
	{
		m_State->zkey.flipSideToMove();
	}

	i += 2;
	m_State->castlingRights = 0;
	if (epd[i] == '-')
	{
		i++;
	}
	else
	{
		if (epd[i] == 'K')
		{
			i++;
			m_State->castlingRights |= 1;
		}

		if (epd[i] == 'Q')
		{
			i++;
			m_State->castlingRights |= 2;
		}

		if (epd[i] == 'k')
		{
			i++;
			m_State->castlingRights |= 4;
		}

		if (epd[i] == 'q')
		{
			i++;
			m_State->castlingRights |= 8;
		}
	}

	m_State->zkey.updateCastlingRights(m_State->castlingRights);

	i++;

	if (epd[i] != '-')
	{
		m_State->epSquare = epd[i] - 'a';
		m_State->epSquare |= (epd[++i] - '1') << 3;
		m_State->zkey.updateEP(m_State->epSquare & 7);
	}
	else
	{
		m_State->epSquare = -1;
	}
	i += 2;

	m_State->halfMoveClock = 0;
	m_GamePly = 0;

	updateCheckInfo();
}

void Board::setState(const Board& other, BoardState& currState, BoardState& rootState)
{
	memcpy(m_Squares, other.m_Squares, 64 + 72);
	m_SideToMove = other.m_SideToMove;
	m_EvalState = other.m_EvalState;
	m_GamePly = other.m_GamePly;
	m_State = &currState;
	m_RootState = &rootState;
}

constexpr char pieceChars[16] = {
    ' ', 'P', 'N', 'B', 'R', 'Q', 'K', '#',
    ' ', 'p', 'n', 'b', 'r', 'q', 'k', '&'
};

std::string Board::stringRep() const
{
    
	std::string result;
	const char* between = "+---+---+---+---+---+---+---+---+\n";

	for (int j = 56; j >= 0; j -= 8)
	{
		result += between;
		for (int i = j; i < j + 8; i++)
		{
			result += "| ";
			Piece piece = m_Squares[i];
			result += pieceChars[static_cast<int>(piece)];
			result += " ";
		}
		result += "|\n";
	}
	result += between;
	return result;
}

std::string Board::fenStr() const
{
	std::string fen = "";
	int lastFile;
	for (int j = 56; j >= 0; j -= 8)
	{
		lastFile = -1;
		for (int i = j; i < j + 8; i++)
		{
			Piece piece = m_Squares[i];
			if (piece != PIECE_NONE)
			{
				int diff = i - j - lastFile;
				if (diff > 1)
					fen += static_cast<char>((diff - 1) + '0');
				fen += pieceChars[static_cast<int>(piece)];
				lastFile = i - j;
			}
		}
		int diff = 8 - lastFile;
		if (diff > 1)
			fen += static_cast<char>((diff - 1) + '0');
		if (j != 0)
			fen += '/';
	}

	fen += ' ';

	fen += m_SideToMove == Color::WHITE ? "w " : "b ";

	if (m_State->castlingRights == 0)
		fen += '-';
	else
	{
		if (m_State->castlingRights & 1)
			fen += 'K';
		if (m_State->castlingRights & 2)
			fen += 'Q';
		if (m_State->castlingRights & 4)
			fen += 'k';
		if (m_State->castlingRights & 8)
			fen += 'q';
	}

	fen += ' ';

	if (m_State->epSquare == -1)
		fen += '-';
	else
	{
		fen += static_cast<char>((m_State->epSquare & 7) + 'a');
		fen += static_cast<char>((m_State->epSquare >> 3) + '1');
	}

	fen += ' ';
	fen += std::to_string(m_State->halfMoveClock);
	fen += ' ';
	fen += std::to_string(m_GamePly / 2 + (m_SideToMove == Color::BLACK));

	return fen;
}

std::string Board::epdStr() const
{
	std::string epd = "";
	int lastFile;
	for (int j = 56; j >= 0; j -= 8)
	{
		lastFile = -1;
		for (int i = j; i < j + 8; i++)
		{
			Piece piece = m_Squares[i];
			if (piece != PIECE_NONE)
			{
				int diff = i - j - lastFile;
				if (diff > 1)
					epd += static_cast<char>((diff - 1) + '0');
				epd += pieceChars[static_cast<int>(piece)];
				lastFile = i - j;
			}
		}
		int diff = 8 - lastFile;
		if (diff > 1)
			epd += static_cast<char>((diff - 1) + '0');
		if (j != 0)
			epd += '/';
	}

	epd += ' ';

	epd += m_SideToMove == Color::WHITE ? "w " : "b ";

	if (m_State->castlingRights == 0)
		epd += '-';
	else
	{
		if (m_State->castlingRights & 1)
			epd += 'K';
		if (m_State->castlingRights & 2)
			epd += 'Q';
		if (m_State->castlingRights & 4)
			epd += 'k';
		if (m_State->castlingRights & 8)
			epd += 'q';
	}

	epd += ' ';

	if (m_State->epSquare == -1)
		epd += '-';
	else
	{
		epd += static_cast<char>((m_State->epSquare & 7) + 'a');
		epd += static_cast<char>((m_State->epSquare >> 3) + '1');
	}

	return epd;
}

void Board::makeMove(Move move, BoardState& state)
{
	state.prev = m_State;
	BoardState* prev = m_State;
	m_State = &state;

	m_State->halfMoveClock = prev->halfMoveClock + 1;
	m_State->pliesFromNull = prev->pliesFromNull + 1;
	m_State->epSquare = prev->epSquare;
	m_State->castlingRights = prev->castlingRights;
	m_State->zkey = prev->zkey;
	m_State->pawnKey = prev->pawnKey;
	m_State->capturedPiece = PIECE_NONE;

	m_GamePly++;

	m_State->zkey.flipSideToMove();

	if (m_State->epSquare != -1)
	{
		m_State->zkey.updateEP(m_State->epSquare & 7);
	}

	switch (move.type())
	{
		case MoveType::NONE:
		{
			Piece srcPiece = m_Squares[move.srcPos()];

			Piece dstPiece = m_Squares[move.dstPos()];
			m_State->capturedPiece = dstPiece;

			if (dstPiece != PIECE_NONE)
			{
				m_State->halfMoveClock = 0;
				removePiece(move.dstPos());
				m_State->zkey.removePiece(getPieceType(dstPiece), getPieceColor(dstPiece), move.dstPos());
				if (getPieceType(dstPiece) == PieceType::PAWN)
					m_State->pawnKey.removePiece(getPieceType(dstPiece), getPieceColor(dstPiece), move.dstPos());
			}

			movePiece(move.srcPos(), move.dstPos());
			m_State->zkey.movePiece(getPieceType(srcPiece), getPieceColor(srcPiece), move.srcPos(), move.dstPos());

			if (getPieceType(srcPiece) == PieceType::PAWN)
			{
				m_State->pawnKey.movePiece(getPieceType(srcPiece), getPieceColor(srcPiece), move.srcPos(), move.dstPos());
				m_State->halfMoveClock = 0;
				if (abs(move.srcPos() - move.dstPos()) == 16)
				{
					m_State->epSquare = (move.srcPos() + move.dstPos()) / 2;
				}
			}
			break;
		}
		case MoveType::PROMOTION:
		{
			m_State->halfMoveClock = 0;

			Piece dstPiece = m_Squares[move.dstPos()];
			m_State->capturedPiece = dstPiece;

			if (dstPiece != PIECE_NONE)
			{
				removePiece(move.dstPos());
				m_State->zkey.removePiece(getPieceType(dstPiece), getPieceColor(dstPiece), move.dstPos());
			}
			removePiece(move.srcPos());
			m_State->zkey.removePiece(PieceType::PAWN, m_SideToMove, move.srcPos());
			m_State->pawnKey.removePiece(PieceType::PAWN, m_SideToMove, move.srcPos());
			addPiece(move.dstPos(), m_SideToMove, promoPiece(move.promotion()));
			m_State->zkey.addPiece(promoPiece(move.promotion()), m_SideToMove, move.dstPos());
			break;
		}
		case MoveType::CASTLE:
		{
			if (move.srcPos() > move.dstPos())
			{
				// queen side
				movePiece(move.srcPos(), move.dstPos());
				m_State->zkey.movePiece(PieceType::KING, m_SideToMove, move.srcPos(), move.dstPos());
				movePiece(move.dstPos() - 2, move.srcPos() - 1);
				m_State->zkey.movePiece(PieceType::ROOK, m_SideToMove, move.dstPos() - 2, move.srcPos() - 1);
			}
			else
			{
				// king side
				movePiece(move.srcPos(), move.dstPos());
				m_State->zkey.movePiece(PieceType::KING, m_SideToMove, move.srcPos(), move.dstPos());
				movePiece(move.dstPos() + 1, move.srcPos() + 1);
				m_State->zkey.movePiece(PieceType::ROOK, m_SideToMove, move.dstPos() + 1, move.srcPos() + 1);
			}
			break;
		}
		case MoveType::ENPASSANT:
		{
			m_State->halfMoveClock = 0;

			int offset = m_SideToMove == Color::WHITE ? -8 : 8;
			m_State->capturedPiece = makePiece(PieceType::PAWN, flip(m_SideToMove));
			removePiece(move.dstPos() + offset);
			m_State->zkey.removePiece(PieceType::PAWN, flip(m_SideToMove), move.dstPos() + offset);
			m_State->pawnKey.removePiece(PieceType::PAWN, flip(m_SideToMove), move.dstPos() + offset);
			movePiece(move.srcPos(), move.dstPos());
			m_State->zkey.movePiece(PieceType::PAWN, m_SideToMove, move.srcPos(), move.dstPos());
			m_State->pawnKey.movePiece(PieceType::PAWN, m_SideToMove, move.srcPos(), move.dstPos());
			break;
		}
	}

	m_State->zkey.updateCastlingRights(m_State->castlingRights);

	m_State->castlingRights &= attacks::getCastleMask(move.srcPos());
	m_State->castlingRights &= attacks::getCastleMask(move.dstPos());

	m_State->zkey.updateCastlingRights(m_State->castlingRights);



	if (m_State->epSquare == prev->epSquare)
	{
		m_State->epSquare = -1;
	}

	if (m_State->epSquare != -1)
	{
		m_State->zkey.updateEP(m_State->epSquare & 7);
	}

	m_SideToMove = flip(m_SideToMove);

	updateCheckInfo();
	calcRepetitions();
}

void Board::unmakeMove(Move move)
{
	m_GamePly--;

	m_SideToMove = flip(m_SideToMove);

	switch (move.type())
	{
		case MoveType::NONE:
		{
			movePiece(move.dstPos(), move.srcPos());
			if (m_State->capturedPiece != PIECE_NONE)
				addPiece(move.dstPos(), m_State->capturedPiece);
			break;
		}
		case MoveType::PROMOTION:
		{
			removePiece(move.dstPos());
			if (m_State->capturedPiece != PIECE_NONE)
				addPiece(move.dstPos(), m_State->capturedPiece);
			addPiece(move.srcPos(), m_SideToMove, PieceType::PAWN);
			break;
		}
		case MoveType::CASTLE:
		{
			if (move.srcPos() > move.dstPos())
			{
				// queen side
				movePiece(move.dstPos(), move.srcPos());
				movePiece(move.srcPos() - 1, move.dstPos() - 2);
			}
			else
			{
				// king side
				movePiece(move.dstPos(), move.srcPos());
				movePiece(move.srcPos() + 1, move.dstPos() + 1);
			}
			break;
		}
		case MoveType::ENPASSANT:
		{
			int offset = m_SideToMove == Color::WHITE ? -8 : 8;
			addPiece(move.dstPos() + offset, m_State->capturedPiece);
			movePiece(move.dstPos(), move.srcPos());
			break;
		}
	}

	m_State = m_State->prev;
}

void Board::makeNullMove(BoardState& state)
{
	state.prev = m_State;
	BoardState* prev = m_State;
	m_State = &state;

	m_State->halfMoveClock = prev->halfMoveClock + 1;
	m_State->pliesFromNull = 0;
	m_State->epSquare = -1;
	m_State->castlingRights = prev->castlingRights;
	m_State->zkey = prev->zkey;
	m_State->pawnKey = prev->pawnKey;
	m_State->repetitions = 0;
	m_State->lastRepetition = 0;

	m_State->capturedPiece = PIECE_NONE;

	m_GamePly++;

	if (prev->epSquare != -1)
	{
		m_State->zkey.updateEP(prev->epSquare & 7);
	}

	m_State->zkey.flipSideToMove();
	m_SideToMove = flip(m_SideToMove);

	updateCheckInfo();
}

void Board::unmakeNullMove()
{
	m_GamePly--;

	m_State = m_State->prev;

	m_SideToMove = flip(m_SideToMove);
}

bool Board::squareAttacked(Color color, uint32_t square, BitBoard blockers) const
{
	return attackersTo(color, square, blockers) != 0;
}

BitBoard Board::attackersTo(Color color, uint32_t square, BitBoard blockers) const
{
	BitBoard queens = getPieces(PieceType::QUEEN);
	BitBoard pawns = (getPieces(color, PieceType::PAWN) & attacks::getPawnAttacks(flip(color), square));
	BitBoard nonPawns =
		(getPieces(PieceType::KNIGHT) & attacks::getKnightAttacks(square)) |
		(getPieces(PieceType::KING) & attacks::getKingAttacks(square)) |
		((getPieces(PieceType::BISHOP) | queens) & attacks::getBishopAttacks(square, blockers)) |
		((getPieces(PieceType::ROOK) | queens) & attacks::getRookAttacks(square, blockers));
	return pawns | (nonPawns & getColor(color));
}

BitBoard Board::attackersTo(uint32_t square, BitBoard blockers) const
{
	BitBoard queens = getPieces(PieceType::QUEEN);
	BitBoard pawns =
		(getPieces(Color::WHITE, PieceType::PAWN) & attacks::getPawnAttacks(Color::BLACK, square)) |
		(getPieces(Color::BLACK, PieceType::PAWN) & attacks::getPawnAttacks(Color::WHITE, square));

	BitBoard nonPawns =
		(getPieces(PieceType::KNIGHT) & attacks::getKnightAttacks(square)) |
		(getPieces(PieceType::KING) & attacks::getKingAttacks(square)) |
		((getPieces(PieceType::BISHOP) | queens) & attacks::getBishopAttacks(square, blockers)) |
		((getPieces(PieceType::ROOK) | queens) & attacks::getRookAttacks(square, blockers));
	return pawns | nonPawns;
}

BitBoard Board::pinnersBlockers(uint32_t square, BitBoard attackers, BitBoard& pinners) const
{
	BitBoard queens = getPieces(PieceType::QUEEN);
	attackers &=
		(attacks::getRookAttacks(square, 0) & (getPieces(PieceType::ROOK) | queens)) |
		(attacks::getBishopAttacks(square, 0) & (getPieces(PieceType::BISHOP) | queens));

	BitBoard blockers = 0;

	BitBoard blockMask = getAllPieces() ^ attackers;

	BitBoard sameColor = getColor(getPieceColor(m_Squares[square]));

	while (attackers)
	{
		uint32_t attacker = popLSB(attackers);

		BitBoard between = attacks::inBetweenBB(square, attacker) & blockMask;

		if (between && (between & (between - 1)) == 0)
		{
			blockers |= between;
			if (between & sameColor)
				pinners |= 1ull << attacker;
		}
	}
	return blockers;
}

// mostly from stockfish
bool Board::see_margin(Move move, int margin) const
{
	int src = move.srcPos();
	int dst = move.dstPos();

	BitBoard allPieces = getAllPieces() ^ (1ull << src);
	BitBoard attackers = attackersTo(dst, allPieces) ^ (1ull << src);

	int value = 0;
	switch (move.type())
	{
		case MoveType::CASTLE:
			// rook and king cannot be attacked after castle
			return 0 >= margin;
		case MoveType::NONE:
		{
			PieceType type = getPieceType(getPieceAt(dst));
			if (type == PieceType::NONE)
				value = -margin;
			else
				value = seePieceValue(type) - margin;
			if (value < 0)
				return false;
			value = seePieceValue(getPieceType(getPieceAt(src))) - value;
			break;
		}
		case MoveType::PROMOTION:
		{
			PieceType promo = promoPiece(move.promotion());
			PieceType type = getPieceType(getPieceAt(dst));
			value = seePieceValue(promo) - seePieceValue(PieceType::PAWN);
			if (type == PieceType::NONE)
				value -= margin;
			else
				value += seePieceValue(type) - margin;
			if (value < 0)
				return false;

			value = seePieceValue(promo) - value;
			break;
		}
		case MoveType::ENPASSANT:
			value = seePieceValue(PieceType::PAWN) - margin;
			if (value < 0)
				return false;
			value = seePieceValue(PieceType::PAWN) - value;
			break;
	}

	if (value <= 0)
		return true;

	Color sideToMove = m_SideToMove;

	BitBoard diagPieces = getPieces(PieceType::BISHOP) | getPieces(PieceType::QUEEN);
	BitBoard straightPieces = getPieces(PieceType::ROOK) | getPieces(PieceType::QUEEN);

	bool us = false;

	while (true)
	{
		sideToMove = flip(sideToMove);
		BitBoard stmAttackers = attackers & getColor(sideToMove);
		if (!stmAttackers)
			return !us;

		if (BitBoard pawns = (stmAttackers & getPieces(PieceType::PAWN)))
		{
			BitBoard pawn = extractLSB(pawns);
			allPieces ^= pawn;
			attackers ^= pawn;
			attackers |= (attacks::getBishopAttacks(dst, allPieces) & allPieces & diagPieces);

			value = seePieceValue(PieceType::PAWN) - value;
			if (value < static_cast<int>(us))
				return us;
		}
		else if (BitBoard knights = (stmAttackers & getPieces(PieceType::KNIGHT)))
		{
			BitBoard knight = extractLSB(knights);
			allPieces ^= knight;
			attackers ^= knight;

			value = seePieceValue(PieceType::KNIGHT) - value;
			if (value < static_cast<int>(us))
				return us;
		}
		else if (BitBoard bishops = (stmAttackers & getPieces(PieceType::BISHOP)))
		{
			BitBoard bishop = extractLSB(bishops);
			allPieces ^= bishop;
			attackers ^= bishop;
			attackers |= (attacks::getBishopAttacks(dst, allPieces) & allPieces & diagPieces);

			value = seePieceValue(PieceType::BISHOP) - value;
			if (value < static_cast<int>(us))
				return us;
		}
		else if (BitBoard rooks = (stmAttackers & getPieces(PieceType::ROOK)))
		{
			BitBoard rook = extractLSB(rooks);
			allPieces ^= rook;
			attackers ^= rook;
			attackers |= (attacks::getRookAttacks(dst, allPieces) & allPieces & straightPieces);

			value = seePieceValue(PieceType::ROOK) - value;
			if (value < static_cast<int>(us))
				return us;
		}
		else if (BitBoard queens = (stmAttackers & getPieces(PieceType::QUEEN)))
		{
			BitBoard queen = extractLSB(queens);
			allPieces ^= queen;
			attackers ^= queen;
			attackers |=
				(attacks::getBishopAttacks(dst, allPieces) & allPieces & diagPieces) |
				(attacks::getRookAttacks(dst, allPieces) & allPieces & straightPieces);

			value = seePieceValue(PieceType::QUEEN) - value;
			if (value < static_cast<int>(us))
				return us;
		}
		else if (stmAttackers & getPieces(PieceType::KING))
		{
			if (attackers & getColor(flip(sideToMove)))
			{
				return !us;
			}
			else
			{
				return us;
			}
		}

		us = !us;
	}

	return us;
}

void Board::updateCheckInfo()
{
	uint32_t whiteKingIdx = getLSB(getPieces(Color::WHITE, PieceType::KING));
	uint32_t blackKingIdx = getLSB(getPieces(Color::BLACK, PieceType::KING));

	m_State->checkInfo.checkers = attackersTo(flip(m_SideToMove), m_SideToMove == Color::WHITE ? whiteKingIdx : blackKingIdx);
	m_State->checkInfo.blockers[static_cast<int>(Color::WHITE)] =
		pinnersBlockers(whiteKingIdx, getColor(Color::BLACK), m_State->checkInfo.pinners[static_cast<int>(Color::WHITE)]);
	m_State->checkInfo.blockers[static_cast<int>(Color::BLACK)] =
		pinnersBlockers(blackKingIdx, getColor(Color::WHITE), m_State->checkInfo.pinners[static_cast<int>(Color::BLACK)]);
}

void Board::calcRepetitions()
{
	int reversible = std::min(m_State->halfMoveClock, m_State->pliesFromNull);
	BoardState* state = m_State;
	for (int i = 2; i <= reversible; i += 2) {
		state = state->prev->prev;
		if (state->zkey == m_State->zkey)
		{
			m_State->repetitions = state->repetitions + 1;
			m_State->lastRepetition = i;
			return;
		}
	}
	m_State->repetitions = 0;
	m_State->lastRepetition = 0;
}

void Board::addPiece(int pos, Color color, PieceType piece)
{
	m_Squares[pos] = makePiece(piece, color);

	BitBoard posBB = 1ull << pos;
	m_Pieces[static_cast<int>(piece)] |= posBB;
	m_Colors[static_cast<int>(color)] |= posBB;
	m_Pieces[static_cast<int>(PieceType::ALL)] |= posBB;

	m_EvalState.addPiece(color, piece, pos);
}

void Board::addPiece(int pos, Piece piece)
{
	m_Squares[pos] = piece;
	BitBoard posBB = 1ull << pos;
	m_Pieces[static_cast<int>(getPieceType(piece))] |= posBB;
	m_Colors[static_cast<int>(getPieceColor(piece))] |= posBB;
	m_Pieces[static_cast<int>(PieceType::ALL)] |= posBB;

	m_EvalState.addPiece(getPieceColor(piece), getPieceType(piece), pos);
}

void Board::removePiece(int pos)
{
	BitBoard posBB = 1ull << pos;
	PieceType pieceType = getPieceType(m_Squares[pos]);
	Color color = getPieceColor(m_Squares[pos]);
	m_Squares[pos] = PIECE_NONE;
	m_Pieces[static_cast<int>(pieceType)] ^= posBB;
	m_Colors[static_cast<int>(color)] ^= posBB;
	m_Pieces[static_cast<int>(PieceType::ALL)] ^= posBB;

	m_EvalState.removePiece(color, pieceType, pos);
}

void Board::movePiece(int src, int dst)
{
	BitBoard srcBB = 1ull << src;
	BitBoard dstBB = 1ull << dst;
	BitBoard moveBB = srcBB | dstBB;
	PieceType pieceType = getPieceType(m_Squares[src]);
	Color color = getPieceColor(m_Squares[src]);


	m_Squares[dst] = m_Squares[src];
	m_Squares[src] = PIECE_NONE;
	m_Pieces[static_cast<int>(pieceType)] ^= moveBB;
	m_Colors[static_cast<int>(color)] ^= moveBB;
	m_Pieces[static_cast<int>(PieceType::ALL)] ^= moveBB;

	m_EvalState.movePiece(color, pieceType, src, dst);
}

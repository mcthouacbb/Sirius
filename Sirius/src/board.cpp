#include "board.h"
#include "attacks.h"

#include <cstring>
#include <charconv>

Board::Board()
{
	setToFen(defaultFen);
}

void Board::setToFen(const std::string_view& fen)
{
	m_State = &m_RootState;
	m_State->pliesFromNull = 0;
	m_State->repetitions = 0;
	m_State->lastRepetition = 0;

	
	memset(m_Squares, 0, 64 + 72);
	m_EvalState.init();
	m_State->zkey.value = 0;
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

const char pieceChars[16] = {
	' ', 'K', 'Q', 'R', 'B', 'N', 'P', '#',
	' ', 'k', 'q', 'r', 'b', 'n', 'p', '&'
};

// maybe include these
/*const char* pieceChars[16] = {
	" ", "♔", "♕", "♖", "♗", "♘", "♙", "#",
	" ", "♚", "♛", "♜", "♝", "♞", "♟", "&"
};*/

void Board::printDbg() const
{
	const char* between = "+---+---+---+---+---+---+---+---+\n";

	for (int j = 56; j >= 0; j -= 8)
	{
		std::cout << between;
		for (int i = j; i < j + 8; i++)
		{
			std::cout << "| ";
			Piece piece = m_Squares[i];
			std::cout << pieceChars[piece];
			std::cout << " ";
		}
		std::cout << "|\n";
	}
	std::cout << between << std::endl;

	std::cout << "All:\n";
	printBB(m_Pieces[0]);

	std::cout << "White:\n";
	printBB(m_Colors[0]);
	std::cout << "Black:\n";
	printBB(m_Colors[1]);

	std::cout << "King:\n";
	printBB(m_Pieces[1]);
	std::cout << "Queen:\n";
	printBB(m_Pieces[2]);
	std::cout << "Rook:\n";
	printBB(m_Pieces[3]);
	std::cout << "Bishop:\n";
	printBB(m_Pieces[4]);
	std::cout << "Knight:\n";
	printBB(m_Pieces[5]);
	std::cout << "Pawn:\n";
	printBB(m_Pieces[6]);
}

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
			result += pieceChars[piece];
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
			if (piece)
			{
				int diff = i - j - lastFile;
				if (diff > 1)
					fen += (diff - 1) + '0';
				fen += pieceChars[piece];
				lastFile = i - j;
			}
		}
		int diff = 8 - lastFile;
		if (diff > 1)
			fen += (diff - 1) + '0';
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
		fen += (m_State->epSquare & 7) + 'a';
		fen += (m_State->epSquare >> 3) + '1';
	}

	fen += ' ';
	fen += std::to_string(m_State->halfMoveClock);
	fen += ' ';
	fen += std::to_string(m_GamePly / 2 + 1 + (m_SideToMove == Color::BLACK));

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
			if (piece)
			{
				int diff = i - j - lastFile;
				if (diff > 1)
					epd += (diff - 1) + '0';
				epd += pieceChars[piece];
				lastFile = i - j;
			}
		}
		int diff = 8 - lastFile;
		if (diff > 1)
			epd += (diff - 1) + '0';
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
		epd += (m_State->epSquare & 7) + 'a';
		epd += (m_State->epSquare >> 3) + '1';
	}
	
	return epd;
}

void Board::makeMove(Move move, BoardState& state)
{
	state.prev = m_State;
	BoardState* prev = m_State;
	m_State = &state;

	/*state.halfMoveClock = m_HalfMoveClock;
	state.reversiblePly = m_ReversiblePly;
	state.pliesFromNull = m_PliesFromNull;
	state.epSquare = m_EpSquare;
	state.castlingRights = m_CastlingRights;
	state.zkey = m_ZKey;
	state.checkInfo = m_CheckInfo;
	state.capturedPiece = 0;*/

	m_State->halfMoveClock = prev->halfMoveClock + 1;
	m_State->pliesFromNull = prev->pliesFromNull + 1;
	m_State->epSquare = prev->epSquare;
	m_State->castlingRights = prev->castlingRights;
	m_State->zkey = prev->zkey;
	m_State->capturedPiece = 0;

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

			if (dstPiece)
			{
				m_State->halfMoveClock = 0;
				removePiece(move.dstPos());
				m_State->zkey.removePiece(static_cast<PieceType>(dstPiece & PIECE_TYPE_MASK), static_cast<Color>(dstPiece >> 3), move.dstPos());
			}

			movePiece(move.srcPos(), move.dstPos());
			m_State->zkey.movePiece(static_cast<PieceType>(srcPiece & PIECE_TYPE_MASK), static_cast<Color>(srcPiece >> 3), move.srcPos(), move.dstPos());

			if ((srcPiece & PIECE_TYPE_MASK) == static_cast<int>(PieceType::PAWN))
			{
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

			if (dstPiece)
			{
				removePiece(move.dstPos());
				m_State->zkey.removePiece(static_cast<PieceType>(dstPiece & PIECE_TYPE_MASK), static_cast<Color>(dstPiece >> 3), move.dstPos());
			}

			const PieceType promoPieces[4] = {
				PieceType::QUEEN,
				PieceType::ROOK,
				PieceType::BISHOP,
				PieceType::KNIGHT
			};
			removePiece(move.srcPos());
			m_State->zkey.removePiece(PieceType::PAWN, m_SideToMove, move.srcPos());
			addPiece(move.dstPos(), m_SideToMove, promoPieces[static_cast<int>(move.promotion()) >> 14]);
			m_State->zkey.addPiece(promoPieces[static_cast<int>(move.promotion()) >> 14], m_SideToMove, move.dstPos());
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
			int col = static_cast<int>(flip(m_SideToMove)) << 3;
			m_State->capturedPiece = col | static_cast<int>(PieceType::PAWN);
			removePiece(move.dstPos() + offset);
			m_State->zkey.removePiece(PieceType::PAWN, flip(m_SideToMove), move.dstPos() + offset);
			movePiece(move.srcPos(), move.dstPos());
			m_State->zkey.movePiece(PieceType::PAWN, m_SideToMove, move.srcPos(), move.dstPos());
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

	/*if (m_Colors[static_cast<int>(Color::WHITE)] & (1ull << 49))
	{
		std::cout << "HELLO?" << std::endl;
		yesnt();
	}*/
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
			if (m_State->capturedPiece)
				addPiece(move.dstPos(), m_State->capturedPiece);
			break;
		}
		case MoveType::PROMOTION:
		{
			removePiece(move.dstPos());
			if (m_State->capturedPiece)
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

	/*if (m_Colors[static_cast<int>(Color::WHITE)] & (1ull << 49))
	{
		std::cout << "HELLO?" << std::endl;
		yesnt();
	}*/
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
	m_State->repetitions = 0;
	m_State->lastRepetition = 0;
	
	m_State->capturedPiece = 0;

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
	BitBoard queens = getPieces(color, PieceType::QUEEN);
	return
		(getPieces(color, PieceType::KNIGHT) & attacks::getKnightAttacks(square)) ||
		(getPieces(color, PieceType::PAWN) & attacks::getPawnAttacks(flip(color), square)) ||
		(getPieces(color, PieceType::KING) & attacks::getKingAttacks(square)) ||
		((getPieces(color, PieceType::BISHOP) | queens) & attacks::getBishopAttacks(square, blockers)) ||
		((getPieces(color, PieceType::ROOK) | queens) & attacks::getRookAttacks(square, blockers));
}

BitBoard Board::attackersTo(Color color, uint32_t square, BitBoard blockers) const
{
	BitBoard queens = getPieces(color, PieceType::QUEEN);
	return
		(getPieces(color, PieceType::KNIGHT) & attacks::getKnightAttacks(square)) |
		(getPieces(color, PieceType::PAWN) & attacks::getPawnAttacks(flip(color), square)) |
		(getPieces(color, PieceType::KING) & attacks::getKingAttacks(square)) |
		((getPieces(color, PieceType::BISHOP) | queens) & attacks::getBishopAttacks(square, blockers)) |
		((getPieces(color, PieceType::ROOK) | queens) & attacks::getRookAttacks(square, blockers));
}

BitBoard Board::pinnersBlockers(uint32_t square, BitBoard attackers, BitBoard& pinners) const
{
	BitBoard queens = getPieces(PieceType::QUEEN);
	attackers &=
		(attacks::getRookAttacks(square, 0) & (getPieces(PieceType::ROOK) | queens)) |
		(attacks::getBishopAttacks(square, 0) & (getPieces(PieceType::BISHOP) | queens));

	BitBoard blockers = 0;

	BitBoard blockMask = getAllPieces() ^ attackers;

	BitBoard sameColor = getColor(static_cast<Color>(m_Squares[square] >> 3));

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

bool Board::givesCheck(Move move) const
{
	uint32_t oppKingIdx = getLSB(getPieces(flip(m_SideToMove), PieceType::KING));
	switch (move.type())
	{
		case MoveType::NONE:
		{
			int piece = m_Squares[move.srcPos()] & PIECE_TYPE_MASK;
			int dstPos = move.dstPos();
			BitBoard checkSqrs = checkSquares(static_cast<PieceType>(piece));
			if (checkSqrs & (1ull << dstPos))
				return true;

			return (checkBlockers(flip(m_SideToMove)) & (1ull << move.srcPos())) &&
				!(attacks::aligned(move.srcPos(), move.dstPos(), oppKingIdx));
		}
		case MoveType::ENPASSANT:
		{
			int dstPos = move.dstPos();
			BitBoard checkSqrs = checkSquares(PieceType::PAWN);
			if (checkSqrs & (1ull << dstPos))
				return true;
			int offset = m_SideToMove == Color::WHITE ? -8 : 8;
			int capturedPawn = dstPos + offset;

			BitBoard postBB = getAllPieces() ^ (1ull << move.srcPos()) ^ (1ull << move.dstPos()) ^ (1ull << capturedPawn);

			return (attacks::getRookAttacks(oppKingIdx, postBB) & (getPieces(m_SideToMove, PieceType::ROOK) | getPieces(m_SideToMove, PieceType::QUEEN))) ||
				(attacks::getBishopAttacks(oppKingIdx, postBB) & (getPieces(m_SideToMove, PieceType::BISHOP) | getPieces(m_SideToMove, PieceType::QUEEN)));
		}
		case MoveType::CASTLE:
		{
			int dstPos = move.dstPos() + (move.srcPos() > move.dstPos() ? -1 : 1);
			BitBoard checkSqrs = checkSquares(PieceType::ROOK);
			if (checkSqrs & (1ull << dstPos))
				return true;

			// enemy king on back rank special case
			return checkSquares(PieceType::ROOK) & (1ull << move.srcPos()) &&
				attacks::aligned(move.srcPos(), move.dstPos(), oppKingIdx);
		}
		case MoveType::PROMOTION:
		{
			int piece = (static_cast<int>(move.promotion()) >> 14) + static_cast<int>(PieceType::QUEEN);
			int dstPos = move.dstPos();
			BitBoard checkSqrs = checkSquares(static_cast<PieceType>(piece));
			if (checkSqrs & (1ull << dstPos))
				return true;

			if ((checkBlockers(flip(m_SideToMove)) & (1ull << move.srcPos())) &&
				!attacks::aligned(move.srcPos(), move.dstPos(), oppKingIdx))
				return true;

			if (checkSquares(PieceType::ROOK) & (1ull << move.srcPos()) &&
				attacks::aligned(move.srcPos(), move.dstPos(), oppKingIdx) &&
				(move.promotion() == Promotion::ROOK || move.promotion() == Promotion::QUEEN))
			{
				return true;
			}

			return checkSquares(PieceType::BISHOP) & (1ull << move.srcPos()) &&
				attacks::aligned(move.srcPos(), move.dstPos(), oppKingIdx) &&
				(move.promotion() == Promotion::BISHOP || move.promotion() == Promotion::QUEEN);
		}
	}
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

	uint32_t oppKingIdx = m_SideToMove == Color::WHITE ? blackKingIdx : whiteKingIdx;

	m_State->checkInfo.checkSquares[static_cast<int>(PieceType::ROOK) - static_cast<int>(PieceType::QUEEN)] =
		attacks::getRookAttacks(oppKingIdx, getAllPieces());
	m_State->checkInfo.checkSquares[static_cast<int>(PieceType::BISHOP) - static_cast<int>(PieceType::QUEEN)] =
		attacks::getBishopAttacks(oppKingIdx, getAllPieces());
	m_State->checkInfo.checkSquares[static_cast<int>(PieceType::QUEEN) - static_cast<int>(PieceType::QUEEN)] =
		m_State->checkInfo.checkSquares[static_cast<int>(PieceType::ROOK) - static_cast<int>(PieceType::QUEEN)] |
		m_State->checkInfo.checkSquares[static_cast<int>(PieceType::BISHOP) - static_cast<int>(PieceType::QUEEN)];
	m_State->checkInfo.checkSquares[static_cast<int>(PieceType::KNIGHT) - static_cast<int>(PieceType::QUEEN)] =
		attacks::getKnightAttacks(oppKingIdx);
	m_State->checkInfo.checkSquares[static_cast<int>(PieceType::PAWN) - static_cast<int>(PieceType::QUEEN)] =
		attacks::getPawnAttacks(flip(m_SideToMove), oppKingIdx);
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
	int col = static_cast<int>(color) << 3;
	// int col = (color == Color::BLACK ? PIECE_COL_MASK : 0);
	m_Squares[pos] = col | static_cast<int>(piece);

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
	m_Pieces[piece & PIECE_TYPE_MASK] |= posBB;
	m_Colors[piece >> 3] |= posBB;
	m_Pieces[static_cast<int>(PieceType::ALL)] |= posBB;

	m_EvalState.addPiece(static_cast<Color>(piece >> 3), static_cast<PieceType>(piece & PIECE_TYPE_MASK), pos);
}

void Board::removePiece(int pos)
{
	BitBoard posBB = 1ull << pos;
	int piece = m_Squares[pos] & PIECE_TYPE_MASK;
	int color = static_cast<bool>(m_Squares[pos] & PIECE_COL_MASK);
	m_Squares[pos] = PIECE_NONE;
	m_Pieces[piece] ^= posBB;
	m_Colors[color] ^= posBB;
	m_Pieces[static_cast<int>(PieceType::ALL)] ^= posBB;

	m_EvalState.removePiece(static_cast<Color>(color), static_cast<PieceType>(piece), pos);
}

void Board::movePiece(int src, int dst)
{
	BitBoard srcBB = 1ull << src;
	BitBoard dstBB = 1ull << dst;
	BitBoard moveBB = srcBB | dstBB;
	int piece = m_Squares[src] & PIECE_TYPE_MASK;
	int color = static_cast<bool>(m_Squares[src] & PIECE_COL_MASK);


	m_Squares[dst] = m_Squares[src];
	m_Squares[src] = PIECE_NONE;
	m_Pieces[piece] ^= moveBB;
	m_Colors[color] ^= moveBB;
	m_Pieces[static_cast<int>(PieceType::ALL)] ^= moveBB;

	m_EvalState.movePiece(static_cast<Color>(color), static_cast<PieceType>(piece), src, dst);
}
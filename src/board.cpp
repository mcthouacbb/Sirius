#include "board.h"
#include "attacks.h"

#include <cstring>
#include <charconv>

Board::Board()
{
	setToFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

void Board::setToFen(const std::string_view& fen)
{
	memset(m_Squares, 0, 64 + 72);
	m_EvalState.init();
	m_ZKey.value = 0;
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
				m_ZKey.addPiece(PieceType::KING, Color::BLACK, sq);
				addPiece(sq++, Color::BLACK, PieceType::KING);
				break;
			case 'q':
				m_ZKey.addPiece(PieceType::QUEEN, Color::BLACK, sq);
				addPiece(sq++, Color::BLACK, PieceType::QUEEN);
				break;
			case 'r':
				m_ZKey.addPiece(PieceType::ROOK, Color::BLACK, sq);
				addPiece(sq++, Color::BLACK, PieceType::ROOK);
				break;
			case 'b':
				m_ZKey.addPiece(PieceType::BISHOP, Color::BLACK, sq);
				addPiece(sq++, Color::BLACK, PieceType::BISHOP);
				break;
			case 'n':
				m_ZKey.addPiece(PieceType::KNIGHT, Color::BLACK, sq);
				addPiece(sq++, Color::BLACK, PieceType::KNIGHT);
				break;
			case 'p':
				m_ZKey.addPiece(PieceType::PAWN, Color::BLACK, sq);
				addPiece(sq++, Color::BLACK, PieceType::PAWN);
				break;
			case 'K':
				m_ZKey.addPiece(PieceType::KING, Color::WHITE, sq);
				addPiece(sq++, Color::WHITE, PieceType::KING);
				break;
			case 'Q':
				m_ZKey.addPiece(PieceType::QUEEN, Color::WHITE, sq);
				addPiece(sq++, Color::WHITE, PieceType::QUEEN);
				break;
			case 'R':
				m_ZKey.addPiece(PieceType::ROOK, Color::WHITE, sq);
				addPiece(sq++, Color::WHITE, PieceType::ROOK);
				break;
			case 'B':
				m_ZKey.addPiece(PieceType::BISHOP, Color::WHITE, sq);
				addPiece(sq++, Color::WHITE, PieceType::BISHOP);
				break;
			case 'N':
				m_ZKey.addPiece(PieceType::KNIGHT, Color::WHITE, sq);
				addPiece(sq++, Color::WHITE, PieceType::KNIGHT);
				break;
			case 'P':
				m_ZKey.addPiece(PieceType::PAWN, Color::WHITE, sq);
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
	m_CastlingRights = 0;
	if (fen[i] == '-')
	{
		i++;
	}
	else
	{
		if (fen[i] == 'K')
		{
			i++;
			m_CastlingRights |= 1;
		}

		if (fen[i] == 'Q')
		{
			i++;
			m_CastlingRights |= 2;
		}

		if (fen[i] == 'k')
		{
			i++;
			m_CastlingRights |= 4;
		}

		if (fen[i] == 'q')
		{
			i++;
			m_CastlingRights |= 8;
		}
	}

	m_ZKey.updateCastlingRights(m_CastlingRights);

	i++;

	if (fen[i] != '-')
	{
		m_EpSquare = fen[i] - 'a';
		m_EpSquare |= (fen[++i] - '1') << 3;
		m_ZKey.updateEP(m_EpSquare & 7);
	}
	else
	{
		m_EpSquare = -1;
	}
	i += 2;

	auto [ptr, ec] = std::from_chars(&fen[i], fen.end(), m_HalfMoveClock);
	std::from_chars(ptr + 1, fen.end(), m_GamePly);
	m_GamePly = 2 * m_GamePly - 1 - (m_SideToMove == Color::WHITE);

	m_ReversiblePly = 0;
	m_PliesFromNull = 0;
	m_State = nullptr;
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

int Board::repetitions() const
{
	if (!m_State)
		return 0;
	int count = 0;
	BoardState* state = m_State->prev;
	for (int i = m_ReversiblePly - 2; i >= 0; i -= 2)
	{
		if (state->zkey == m_ZKey)
			count++;
		if (state->prev)
			state = state->prev->prev;
	}

	return count;
}

void Board::makeMove(Move move, BoardState& state)
{
	state.prev = m_State;
	m_State = &state;
	
	state.halfMoveClock = m_HalfMoveClock;
	state.reversiblePly = m_ReversiblePly;
	state.pliesFromNull = m_PliesFromNull;
	state.epSquare = m_EpSquare;
	state.castlingRights = m_CastlingRights;
	state.zkey = m_ZKey;
	state.checkInfo = m_CheckInfo;
	state.capturedPiece = 0;
	
	m_GamePly++;
	m_HalfMoveClock++;
	m_ReversiblePly++;
	m_PliesFromNull++;

	m_ZKey.flipSideToMove();

	if (m_EpSquare != -1)
	{
		m_ZKey.updateEP(m_EpSquare & 7);
	}

	switch (move.type())
	{
		case MoveType::NONE:
		{
			Piece srcPiece = m_Squares[move.srcPos()];
			
			Piece dstPiece = m_Squares[move.dstPos()];
			state.capturedPiece = dstPiece;
		
			if (dstPiece)
			{
				m_HalfMoveClock = 0;
				removePiece(move.dstPos());
				m_ZKey.removePiece(static_cast<PieceType>(dstPiece & PIECE_TYPE_MASK), static_cast<Color>(dstPiece >> 3), move.dstPos());
			}
			
			movePiece(move.srcPos(), move.dstPos());
			m_ZKey.movePiece(static_cast<PieceType>(srcPiece & PIECE_TYPE_MASK), static_cast<Color>(srcPiece >> 3), move.srcPos(), move.dstPos());
			
			if ((srcPiece & PIECE_TYPE_MASK) == static_cast<int>(PieceType::PAWN))
			{
				m_HalfMoveClock = 0;
				m_ReversiblePly = 0;
				if (abs(move.srcPos() - move.dstPos()) == 16)
				{
					m_EpSquare = (move.srcPos() + move.dstPos()) / 2;
				}
			}
			break;
		}
		case MoveType::PROMOTION:
		{
			m_HalfMoveClock = 0;
			m_ReversiblePly = 0;

			Piece dstPiece = m_Squares[move.dstPos()];
			state.capturedPiece = dstPiece;
		
			if (dstPiece)
			{
				removePiece(move.dstPos());
				m_ZKey.removePiece(static_cast<PieceType>(dstPiece & PIECE_TYPE_MASK), static_cast<Color>(dstPiece >> 3), move.dstPos());
			}
			
			const PieceType promoPieces[4] = {
				PieceType::QUEEN,
				PieceType::ROOK,
				PieceType::BISHOP,
				PieceType::KNIGHT
			};
			removePiece(move.srcPos());
			m_ZKey.removePiece(PieceType::PAWN, m_SideToMove, move.srcPos());
			addPiece(move.dstPos(), m_SideToMove, promoPieces[static_cast<int>(move.promotion()) >> 14]);
			m_ZKey.addPiece(promoPieces[static_cast<int>(move.promotion()) >> 14], m_SideToMove, move.dstPos());
			break;
		}
		case MoveType::CASTLE:
		{
			m_ReversiblePly = 0;
			if (move.srcPos() > move.dstPos())
			{
				// queen side
				movePiece(move.srcPos(), move.dstPos());
				m_ZKey.movePiece(PieceType::KING, m_SideToMove, move.srcPos(), move.dstPos());
				movePiece(move.dstPos() - 2, move.srcPos() - 1);
				m_ZKey.movePiece(PieceType::ROOK, m_SideToMove, move.dstPos() - 2, move.srcPos() - 1);
			}
			else
			{
				// king side
				movePiece(move.srcPos(), move.dstPos());
				m_ZKey.movePiece(PieceType::KING, m_SideToMove, move.srcPos(), move.dstPos());
				movePiece(move.dstPos() + 1, move.srcPos() + 1);
				m_ZKey.movePiece(PieceType::ROOK, m_SideToMove, move.dstPos() + 1, move.srcPos() + 1);
			}
			break;
		}
		case MoveType::ENPASSANT:
		{	
			m_HalfMoveClock = 0;
			m_ReversiblePly = 0;

			int offset = m_SideToMove == Color::WHITE ? -8 : 8;
			int col = static_cast<int>(flip(m_SideToMove)) << 3;
			state.capturedPiece = col | static_cast<int>(PieceType::PAWN);
			removePiece(move.dstPos() + offset);
			m_ZKey.removePiece(PieceType::PAWN, flip(m_SideToMove), move.dstPos() + offset);
			movePiece(move.srcPos(), move.dstPos());
			m_ZKey.movePiece(PieceType::PAWN, m_SideToMove, move.srcPos(), move.dstPos());
			break;
		}
	}

	m_ZKey.updateCastlingRights(m_CastlingRights);
	
	m_CastlingRights &= attacks::getCastleMask(move.srcPos());
	m_CastlingRights &= attacks::getCastleMask(move.dstPos());

	m_ZKey.updateCastlingRights(m_CastlingRights);

	
	
	if (m_EpSquare == state.epSquare)
	{
		m_EpSquare = -1;
	}

	if (m_EpSquare != -1)
	{
		m_ZKey.updateEP(m_EpSquare & 7);
	}

	m_SideToMove = flip(m_SideToMove);
	
	updateCheckInfo();
	
	/*if (m_Colors[static_cast<int>(Color::WHITE)] & (1ull << 49))
	{
		std::cout << "HELLO?" << std::endl;
		yesnt();
	}*/
}

void Board::unmakeMove(Move move)
{
	m_GamePly--;
	m_HalfMoveClock = m_State->halfMoveClock;
	m_ReversiblePly = m_State->reversiblePly;
	m_PliesFromNull = m_State->pliesFromNull;
	m_EpSquare = m_State->epSquare;
	m_CastlingRights = m_State->castlingRights;
	m_ZKey = m_State->zkey;

	m_CheckInfo = m_State->checkInfo;

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
	m_State = &state;
	
	state.halfMoveClock = m_HalfMoveClock;
	state.reversiblePly = m_ReversiblePly;
	state.pliesFromNull = m_PliesFromNull;
	state.epSquare = m_EpSquare;
	state.castlingRights = m_CastlingRights;
	state.zkey = m_ZKey;
	state.checkInfo = m_CheckInfo;
	state.capturedPiece = 0;

	m_GamePly++;
	m_HalfMoveClock++;
	m_ReversiblePly++;
	
	m_PliesFromNull = 0;
	
	if (m_EpSquare != -1)
	{
		m_ZKey.updateEP(m_EpSquare & 7);
		m_EpSquare = -1;
	}

	m_ZKey.flipSideToMove();
	m_SideToMove = flip(m_SideToMove);

	updateCheckInfo();
}

void Board::unmakeNullMove()
{
	m_GamePly--;
	m_HalfMoveClock = m_State->halfMoveClock;
	m_ReversiblePly = m_State->reversiblePly;
	m_PliesFromNull = m_State->pliesFromNull;
	m_EpSquare = m_State->epSquare;
	m_CastlingRights = m_State->castlingRights;
	m_ZKey = m_State->zkey;

	m_CheckInfo = m_State->checkInfo;

	m_SideToMove = flip(m_SideToMove);

	m_State = m_State->prev;
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
	
	while (attackers)
	{
		uint32_t attacker = popLSB(attackers);

		BitBoard between = attacks::inBetweenBB(square, attacker) & blockMask;
		
		if (between && (between & (between - 1)) == 0)
		{
			blockers |= between;
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

	m_CheckInfo.checkers = attackersTo(flip(m_SideToMove), m_SideToMove == Color::WHITE ? whiteKingIdx : blackKingIdx);
	m_CheckInfo.blockers[static_cast<int>(Color::WHITE)] =
		pinnersBlockers(whiteKingIdx, getColor(Color::BLACK), m_CheckInfo.pinners[static_cast<int>(Color::WHITE)]);
	m_CheckInfo.blockers[static_cast<int>(Color::BLACK)] =
		pinnersBlockers(blackKingIdx, getColor(Color::WHITE), m_CheckInfo.pinners[static_cast<int>(Color::BLACK)]);

	uint32_t oppKingIdx = m_SideToMove == Color::WHITE ? blackKingIdx : whiteKingIdx;
	
	m_CheckInfo.checkSquares[static_cast<int>(PieceType::ROOK) - static_cast<int>(PieceType::QUEEN)] =
		attacks::getRookAttacks(oppKingIdx, getAllPieces());
	m_CheckInfo.checkSquares[static_cast<int>(PieceType::BISHOP) - static_cast<int>(PieceType::QUEEN)] =
		attacks::getBishopAttacks(oppKingIdx, getAllPieces());
	m_CheckInfo.checkSquares[static_cast<int>(PieceType::QUEEN) - static_cast<int>(PieceType::QUEEN)] =
		m_CheckInfo.checkSquares[static_cast<int>(PieceType::ROOK) - static_cast<int>(PieceType::QUEEN)] |
		m_CheckInfo.checkSquares[static_cast<int>(PieceType::BISHOP) - static_cast<int>(PieceType::QUEEN)];
	m_CheckInfo.checkSquares[static_cast<int>(PieceType::KNIGHT) - static_cast<int>(PieceType::QUEEN)] =
		attacks::getKnightAttacks(oppKingIdx);
	m_CheckInfo.checkSquares[static_cast<int>(PieceType::PAWN) - static_cast<int>(PieceType::QUEEN)] =
		attacks::getPawnAttacks(flip(m_SideToMove), oppKingIdx);
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
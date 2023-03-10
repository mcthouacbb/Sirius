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
				addPiece(sq++, Color::BLACK, PieceType::KING);
				break;
			case 'q':
				addPiece(sq++, Color::BLACK, PieceType::QUEEN);
				break;
			case 'r':
				addPiece(sq++, Color::BLACK, PieceType::ROOK);
				break;
			case 'b':
				addPiece(sq++, Color::BLACK, PieceType::BISHOP);
				break;
			case 'n':
				addPiece(sq++, Color::BLACK, PieceType::KNIGHT);
				break;
			case 'p':
				addPiece(sq++, Color::BLACK, PieceType::PAWN);
				break;
			case 'K':
				addPiece(sq++, Color::WHITE, PieceType::KING);
				break;
			case 'Q':
				addPiece(sq++, Color::WHITE, PieceType::QUEEN);
				break;
			case 'R':
				addPiece(sq++, Color::WHITE, PieceType::ROOK);
				break;
			case 'B':
				addPiece(sq++, Color::WHITE, PieceType::BISHOP);
				break;
			case 'N':
				addPiece(sq++, Color::WHITE, PieceType::KNIGHT);
				break;
			case 'P':
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
	m_CurrPlayer = fen[i] == 'w' ? Color::WHITE : Color::BLACK;

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

	i++;

	if (fen[i] != '-')
	{
		m_Enpassant = fen[i] - 'a';
		m_Enpassant |= (fen[++i] - '1') << 3;
	}
	else
	{
		m_Enpassant = 0;
	}
	i += 2;

	auto [ptr, ec] = std::from_chars(&fen[i], fen.end(), m_HalfMoveClock);
	std::from_chars(ptr + 1, fen.end(), m_GamePly);
	m_GamePly = 2 * m_GamePly - 1 - (m_CurrPlayer == Color::WHITE);
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

void Board::printDbg()
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

void Board::makeMove(const Move move, BoardState& state)
{
	m_GamePly++;
	state.halfMoveClock = m_HalfMoveClock;
	state.epSquare = m_Enpassant;
	state.castlingRights = m_CastlingRights;
	

	switch (move.type())
	{
		case MoveType::NONE:
		{
			state.srcPiece = m_Squares[move.srcPos()];
			
			Piece dstPiece = m_Squares[move.dstPos()];
			state.dstPiece = dstPiece;
		
			if (dstPiece)
			{
				m_HalfMoveClock = 0;
				removePiece(move.dstPos());
			}
			
			movePiece(move.srcPos(), move.dstPos());
			if ((state.srcPiece & PIECE_TYPE_MASK) == static_cast<int>(PieceType::PAWN))
			{
				m_HalfMoveClock = 0;
				if (abs(move.srcPos() - move.dstPos()) == 16)
				{
					m_Enpassant = (move.srcPos() + move.dstPos()) / 2;
				}
			}
			break;
		}
		case MoveType::PROMOTION:
		{
			m_HalfMoveClock = 0;
			state.srcPiece = m_Squares[move.srcPos()];

			Piece dstPiece = m_Squares[move.dstPos()];
			state.dstPiece = dstPiece;
		
			if (dstPiece)
			{
				removePiece(move.dstPos());
			}
			
			const PieceType promoPieces[4] = {
				PieceType::QUEEN,
				PieceType::ROOK,
				PieceType::BISHOP,
				PieceType::KNIGHT
			};
			removePiece(move.srcPos());
			addPiece(move.dstPos(), m_CurrPlayer, promoPieces[static_cast<int>(move.promotion()) >> 14]);
			break;
		}
		case MoveType::CASTLE:
		{
			if (move.srcPos() > move.dstPos())
			{
				// queen side
				movePiece(move.srcPos(), move.dstPos());
				movePiece(move.dstPos() - 2, move.srcPos() - 1);
			}
			else
			{
				// king side
				movePiece(move.srcPos(), move.dstPos());
				movePiece(move.dstPos() + 1, move.srcPos() + 1);
			}
			break;
		}
		case MoveType::ENPASSANT:
		{	
			m_HalfMoveClock = 0;

			state.srcPiece = m_Squares[move.srcPos()];
			
			int offset = m_CurrPlayer == Color::WHITE ? -8 : 8;
			int col = static_cast<int>(flip(m_CurrPlayer)) << 3;
			state.dstPiece = col | static_cast<int>(PieceType::PAWN);
			removePiece(move.dstPos() + offset);
			movePiece(move.srcPos(), move.dstPos());
			break;
		}
	}
	
	m_CastlingRights &= attacks::getCastleMask(move.srcPos());
	m_CastlingRights &= attacks::getCastleMask(move.dstPos());

	if (m_Enpassant == state.epSquare)
		m_Enpassant = 0;

	m_CurrPlayer = flip(m_CurrPlayer);

	/*if (m_Colors[static_cast<int>(Color::WHITE)] & (1ull << 49))
	{
		std::cout << "HELLO?" << std::endl;
		yesnt();
	}*/
}

void Board::unmakeMove(Move move, const BoardState& state)
{
	m_GamePly--;
	m_HalfMoveClock = state.halfMoveClock;
	m_Enpassant = state.epSquare;
	m_CastlingRights = state.castlingRights;

	m_CurrPlayer = flip(m_CurrPlayer);

	switch (move.type())
	{
		case MoveType::NONE:
		{
			movePiece(move.dstPos(), move.srcPos());
			if (state.dstPiece)
				addPiece(move.dstPos(), state.dstPiece);
			break;
		}
		case MoveType::PROMOTION:
		{
			removePiece(move.dstPos());
			if (state.dstPiece)
				addPiece(move.dstPos(), state.dstPiece);
			addPiece(move.srcPos(), state.srcPiece);
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
			int offset = m_CurrPlayer == Color::WHITE ? -8 : 8;
			addPiece(move.dstPos() + offset, state.dstPiece);
			movePiece(move.dstPos(), move.srcPos());
			break;
		}
	}

	if (m_Pieces[0] != (m_Colors[0] | m_Colors[1]))
	{
		printBB(m_Pieces[0] ^ (m_Colors[0] | m_Colors[1]));
		throw std::runtime_error("Impossible");
	}
	
	/*if (m_Colors[static_cast<int>(Color::WHITE)] & (1ull << 49))
	{
		std::cout << "HELLO?" << std::endl;
		yesnt();
	}*/
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
}

void Board::addPiece(int pos, Piece piece)
{
	m_Squares[pos] = piece;
	BitBoard posBB = 1ull << pos;
	m_Pieces[piece & PIECE_TYPE_MASK] |= posBB;
	m_Colors[piece >> 3] |= posBB;
	m_Pieces[static_cast<int>(PieceType::ALL)] |= posBB;
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
	// if (getPopcnt(m_Colors[color]) > 16)
		// yesnt();
	m_Pieces[static_cast<int>(PieceType::ALL)] ^= moveBB;
}
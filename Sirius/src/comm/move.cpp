#include "move.h"

namespace comm
{

const char promoChars[4] = {'q', 'r', 'b', 'n'};
const char pieceChars[5] = {'K', 'Q', 'R', 'B', 'N'};

MoveStrFind findMoveFromPCN(Move* begin, Move* end, const char* moveStr)
{
	int src = (moveStr[0] - 'a') + ((moveStr[1] - '1') << 3);
	int dst = (moveStr[2] - 'a') + ((moveStr[3] - '1') << 3);

	Promotion promotion = Promotion(-1);
	bool isPromotion = false;
	switch (tolower(moveStr[4]))
	{
		case 'q':
			promotion = Promotion::QUEEN;
			isPromotion = true;
			break;
		case 'r':
			promotion = Promotion::ROOK;
			isPromotion = true;
			break;
		case 'b':
			promotion = Promotion::BISHOP;
			isPromotion = true;
			break;
		case 'n':
			promotion = Promotion::KNIGHT;
			isPromotion = true;
			break;
	}

	for (Move* it = begin; it != end; it++)
	{
		if (it->srcPos() == src && it->dstPos() == dst)
		{
			if (isPromotion)
			{
				if (it->type() == MoveType::PROMOTION && it->promotion() == promotion)
					return {it, moveStr + 4 + isPromotion};
			}
			else
			{
				return {it, moveStr + 4 + isPromotion};
			}
		}
	}
	return {nullptr, moveStr + 4 + isPromotion};
}

MoveStrFind findMoveFromSAN(const Board& board, Move* begin, Move* end, const char* moveStr)
{
	int fromFile = -1;
	int fromRank = -1;
	int toFile = -1;
	int toRank = -1;
	PieceType piece = PieceType::NONE;
	Promotion promotion = Promotion(-1);
	bool isPromotion = false;
	bool isCapture = false;

	int moveLen = -1;

	if (moveStr[0] == '\0')
		return {nullptr, moveStr};

	switch (moveStr[0])
	{
		case 'K':
			piece = PieceType::KING;
			goto piece_moves;
		case 'Q':
			piece = PieceType::QUEEN;
			goto piece_moves;
		case 'R':
			piece = PieceType::ROOK;
			goto piece_moves;
		case 'B':
			piece = PieceType::BISHOP;
			goto piece_moves;
		case 'N':
			piece = PieceType::KNIGHT;
			goto piece_moves;
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		case 'g':
		case 'h':
			piece = PieceType::PAWN;
			if (moveStr[1] == 'x')
			{
				isCapture = true;
				fromFile = moveStr[0] - 'a';
			}
			else
			{
				toFile = moveStr[0] - 'a';
			}
			break;
		case 'O':
			if (moveStr[1] != '-')
				return {nullptr, moveStr};
			if (moveStr[2] != 'O')
				return {nullptr, moveStr};
			if (moveStr[3] == '\0')
			{
				for (Move* it = begin; it != end; it++)
				{
					if (it->type() == MoveType::CASTLE && it->dstPos() > it->srcPos())
					{
						return {it, moveStr + 3};
					}
				}
				return {end, moveStr + 3};
			}
			else
			{
				if (moveStr[3] != '-')
					return {nullptr, moveStr};
				if (moveStr[4] != 'O')
					return {nullptr, moveStr};
				if (moveStr[5] != '\0')
					return {nullptr, moveStr};
				for (Move* it = begin; it != end; it++)
				{
					if (it->type() == MoveType::CASTLE && it->dstPos() < it->srcPos())
					{
						return {it, moveStr + 5};
					}
				}
				return {end, moveStr + 5};
			}
		default:
			return {nullptr, moveStr};
	}

	if (isCapture)
	{
		if (moveStr[2] < 'a' || moveStr[2] > 'h' || moveStr[3] < '1' || moveStr[3] > '8')
			return {nullptr, moveStr};
		toFile = moveStr[2] - 'a';
		toRank = moveStr[3] - '1';
		int i = 4;
		if (moveStr[4] == '=')
			i = 5;
		switch (moveStr[i])
		{
			case 'q':
				promotion = Promotion::QUEEN;
				isPromotion = true;
				break;
			case 'r':
				promotion = Promotion::ROOK;
				isPromotion = true;
				break;
			case 'b':
				promotion = Promotion::BISHOP;
				isPromotion = true;
				break;
			case 'n':
				promotion = Promotion::KNIGHT;
				isPromotion = true;
				break;
		}

		moveLen = i + isPromotion;
	}
	else
	{
		if (moveStr[1] < '1' || moveStr[1] > '8')
			return {nullptr, moveStr};
		toRank = moveStr[1] - '1';
		int i = 2;
		if (moveStr[i] == '=')
			i = 3;

		switch (moveStr[i])
		{
			case 'q':
				promotion = Promotion::QUEEN;
				isPromotion = true;
				break;
			case 'r':
				promotion = Promotion::ROOK;
				isPromotion = true;
				break;
			case 'b':
				promotion = Promotion::BISHOP;
				isPromotion = true;
				break;
			case 'n':
				promotion = Promotion::KNIGHT;
				isPromotion = true;
				break;
		}
		moveLen = i + isPromotion;
	}
	goto search_moves;
piece_moves:
	if (moveStr[1] == '\0')
		return {nullptr, moveStr};
	if (moveStr[2] == '\0')
		return {nullptr, moveStr};
	if (moveStr[1] == 'x')
	{
		isCapture = true;
		if (moveStr[2] < 'a' || moveStr[2] > 'h' || moveStr[3] < '1' || moveStr[3] > '8')
			return {nullptr, moveStr};
		toFile = moveStr[2] - 'a';
		toRank = moveStr[3] - '1';
		moveLen = 4;
	}
	else if (moveStr[2] == 'x')
	{
		isCapture = true;
		switch (moveStr[1])
		{
			case 'a':
			case 'b':
			case 'c':
			case 'd':
			case 'e':
			case 'f':
			case 'g':
			case 'h':
				fromFile = moveStr[1] - 'a';
				break;
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
				fromRank = moveStr[1] - '1';
				break;
			default:
				return {nullptr, moveStr};
		}
		if (moveStr[3] < 'a' || moveStr[3] > 'h' || moveStr[4] < '1' || moveStr[4] > '8')
			return {nullptr, moveStr};
		toFile = moveStr[3] - 'a';
		toRank = moveStr[4] - '1';
		moveLen = 5;
	}
	else if (moveStr[3] == 'x')
	{
		if (moveStr[4] < 'a' || moveStr[4] > 'h' || moveStr[5] < '1' || moveStr[5] > '8')
			return {nullptr, moveStr};

		isCapture = true;
		fromFile = moveStr[1] - 'a';
		fromRank = moveStr[2] - '1';

		toFile = moveStr[4] - 'a';
		toRank = moveStr[5] - '1';
		moveLen = 6;
	}
	else
	{
		int file1 = -1;
		int rank1 = -1;
		switch (moveStr[1])
		{
			case 'a':
			case 'b':
			case 'c':
			case 'd':
			case 'e':
			case 'f':
			case 'g':
			case 'h':
				file1 = moveStr[1] - 'a';
				break;
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
				rank1 = moveStr[1] - '1';
				break;
			default:
				return {nullptr, moveStr};
		}

		if (file1 != -1)
		{
			switch (moveStr[2])
			{
				case 'a':
				case 'b':
				case 'c':
				case 'd':
				case 'e':
				case 'f':
				case 'g':
				case 'h':
					toFile = moveStr[2] - 'a';
					break;
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
					rank1 = moveStr[2] - '1';
					break;
				default:
					return {nullptr, moveStr};
			}

			if (toFile != -1)
			{
				if (moveStr[3] < '1' || moveStr[3] > '8')
					return {nullptr, moveStr};
				fromFile = file1;
				toRank = moveStr[3] - '1';
				moveLen = 4;
			}
			else
			{
				if (moveStr[3] >= 'a' && moveStr[3] <= 'h')
				{
					if (moveStr[4] < '1' || moveStr[4] > '8')
						return {nullptr, moveStr};
					fromFile = file1;
					fromRank = rank1;

					toFile = moveStr[3] - 'a';
					toRank = moveStr[4] - '1';
					moveLen = 5;
				}
				else
				{
					toFile = file1;
					toRank = rank1;
					moveLen = 3;
				}
			}
		}
		else
		{
			fromRank = rank1;
			if (moveStr[2] < 'a' || moveStr[2] > 'h' || moveStr[3] < '1' || moveStr[3] > '8')
				return {nullptr, moveStr};
			toFile = moveStr[2] - 'a';
			toRank = moveStr[3] - '1';
			moveLen = 4;
		}
	}
search_moves:
	int toSquare = toFile | (toRank << 3);
	// std::cout << toSquare << std::endl;
	/*std::cout << "From File: " << fromFile << std::endl;
	std::cout << "From Rank: " << fromRank << std::endl;
	std::cout << "To File: " << toFile << std::endl;
	std::cout << "To Rank: " << toRank << std::endl;
	std::cout << "To Square: " << toSquare << std::endl;
	std::cout << "Piece: " << static_cast<int>(piece) << std::endl;
	std::cout << "is promotion: " << isPromotion << std::endl;
	if (isPromotion)
		std::cout << "promotion: " << (static_cast<int>(promotion) >> 14) << std::endl;
	std::cout << "Is capture: " << isCapture << std::endl;

	std::cout << "Move length: " << moveLen << std::endl;

	std::cout << std::endl;*/

	// if (isCapture && !board.getPieceAt(toSquare))
		// return {end, moveStr + moveLen};

	if (!isCapture && board.getPieceAt(toSquare))
	{
		return {end, moveStr + moveLen};
	}

	int pawnOffset = (board.currPlayer() == Color::WHITE) ? -8 : 8;

	Move* match = nullptr;
	for (Move* it = begin; it != end; it++)
	{
		switch (it->type())
		{
			case MoveType::ENPASSANT:
				if (isCapture && (toRank == 2 || toRank == 5) && !board.getPieceAt(toSquare + pawnOffset))
					continue;
				break;
			default:
				if (isCapture && !board.getPieceAt(toSquare))
					continue;
				break;
		}
		if (isPromotion && it->type() != MoveType::PROMOTION)
			continue;
		if (!isPromotion && it->type() == MoveType::PROMOTION)
			continue;

		if (isPromotion && it->promotion() != promotion)
			continue;

		if (it->dstPos() != toSquare)
			continue;

		int srcSquare = it->srcPos();
		Piece srcPiece = board.getPieceAt(srcSquare);
		if ((srcPiece & PIECE_TYPE_MASK) != static_cast<int>(piece))
			continue;

		if (fromRank != -1 && fromRank != (srcSquare >> 3))
			continue;
		if (fromFile != -1 && fromFile != (srcSquare & 7))
			continue;

		if (match)
			return {end + 1, moveStr + moveLen};
		match = it;
	}

	if (match)
		return {match, moveStr + moveLen};
	else
		return {end, moveStr + moveLen};
}


std::string convMoveToPCN(Move move)
{
	std::string str(4 + (move.type() == MoveType::PROMOTION), ' ');
	str[0] = static_cast<char>((move.srcPos() & 7) + 'a');
	str[1] = static_cast<char>((move.srcPos() >> 3) + '1');
	str[2] = static_cast<char>((move.dstPos() & 7) + 'a');
	str[3] = static_cast<char>((move.dstPos() >> 3) + '1');
	if (move.type() == MoveType::PROMOTION)
	{
		str[4] = promoChars[(static_cast<int>(move.promotion()) >> 14)];
	}
	return str;
}

bool isAmbiguous(const Board& board, Move* begin, Move* end, PieceType piece, int toSquare, int fromFile, int fromRank)
{
	Move* match = nullptr;
	for (Move* it = begin; it != end; it++)
	{
		int srcPos = it->srcPos();
		int dstPos = it->dstPos();
		PieceType movePiece = static_cast<PieceType>(board.getPieceAt(srcPos) & PIECE_TYPE_MASK);
		if (movePiece != piece)
			continue;

		if (dstPos != toSquare)
			continue;

		if (fromFile != -1 && fromFile != (srcPos & 7))
			continue;

		if (fromRank != -1 && fromRank != (srcPos >> 3))
			continue;

		if (match)
			return true;
		else
			match = it;
	}
	return false;
}

std::string convMoveToSAN(const Board& board, Move* begin, Move* end, Move move)
{
	if (move.type() == MoveType::CASTLE)
	{
		return move.dstPos() > move.srcPos() ? "O-O" : "O-O-O";
	}
	PieceType piece = static_cast<PieceType>(board.getPieceAt(move.srcPos()) & PIECE_TYPE_MASK);
	bool isCapture = static_cast<bool>(board.getPieceAt(move.dstPos()));
	if (piece == PieceType::PAWN)
	{
		int srcPos = move.srcPos();
		int dstPos = move.dstPos();
		if ((srcPos & 7) == (dstPos & 7))
		{
			std::string str(2 + 2 * (move.type() == MoveType::PROMOTION), ' ');
			if (move.type() == MoveType::PROMOTION)
			{
				str[2] = '=';
				str[3] = promoChars[static_cast<int>(move.promotion()) >> 14];
			}
			str[0] = static_cast<char>((dstPos & 7) + 'a');
			str[1] = static_cast<char>((dstPos >> 3) + '1');
			return str;
		}
		else
		{
			std::string str(4 + 2 * (move.type() == MoveType::PROMOTION), ' ');
			if (move.type() == MoveType::PROMOTION)
			{
				str[4] = '=';
				str[5] = promoChars[static_cast<int>(move.promotion()) >> 14];
			}
			str[0] = static_cast<char>((srcPos & 7) + 'a');
			str[1] = 'x';
			str[2] = static_cast<char>((dstPos & 7) + 'a');
			str[3] = static_cast<char>((dstPos >> 3) + '1');
			return str;
		}
	}
	else
	{
		char pceChar = pieceChars[static_cast<int>(piece) - 1];

		int srcPos = move.srcPos();
		int dstPos = move.dstPos();

		if (!isAmbiguous(board, begin, end, piece, dstPos, -1, -1))
		{
			std::string str(3 + isCapture, ' ');
			str[0] = pceChar;
			if (isCapture)
			{
				str[1] = 'x';
			}
			str[1 + isCapture] = static_cast<char>((dstPos & 7) + 'a');
			str[2 + isCapture] = static_cast<char>((dstPos >> 3) + '1');
			return str;
		}

		if (!isAmbiguous(board, begin, end, piece, dstPos, srcPos & 7, -1))
		{
			std::string str(4 + isCapture, ' ');
			str[0] = pceChar;
			str[1] = static_cast<char>((srcPos & 7) + 'a');
			if (isCapture)
			{
				str[2] = 'x';
			}
			str[2 + isCapture] = static_cast<char>((dstPos & 7) + 'a');
			str[3 + isCapture] = static_cast<char>((dstPos >> 3) + '1');
			return str;
		}

		if (!isAmbiguous(board, begin, end, piece, dstPos, srcPos >> 3, -1))
		{
			std::string str(4 + isCapture, ' ');
			str[0] = pceChar;
			str[1] = static_cast<char>((srcPos >> 3) + '1');
			if (isCapture)
			{
				str[2] = 'x';
			}
			str[2 + isCapture] = static_cast<char>((dstPos & 7) + 'a');
			str[3 + isCapture] = static_cast<char>((dstPos >> 3) + '1');
			return str;
		}

		std::string str(5 + isCapture, ' ');
		str[0] = pceChar;
		str[1] = static_cast<char>((srcPos & 7) + 'a');
		str[2] = static_cast<char>((srcPos >> 3) + '1');
		if (isCapture)
		{
			str[3] = 'x';
		}
		str[3 + isCapture] = static_cast<char>((dstPos & 7) + 'a');
		str[4 + isCapture] = static_cast<char>((dstPos >> 3) + '1');
		return str;
	}
}


}
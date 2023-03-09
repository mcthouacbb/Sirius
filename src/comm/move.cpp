#include "move.h"

namespace comm
{

MoveStrFind findMoveFromPCN(Move* begin, Move* end, const char* moveStr)
{
	int src = (moveStr[0] - 'a') + ((moveStr[1] - '1') << 3);
	int dst = (moveStr[2] - 'a') + ((moveStr[3] - '1') << 3);

	Promotion promotion;
	bool isPromotion = false;
	switch (moveStr[4])
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
	Promotion promotion;
	bool isPromotion = false;
	bool isCapture = false;

	int moveLen = -1;
	
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
			piece = PieceType::KNIGHT;
			goto piece_moves;
		case 'N':
			piece = PieceType::BISHOP;
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
				fromFile = moveStr[0];
			}
			else
			{
				toFile = moveStr[0] - 'a';
			}
			break;
	}

	if (isCapture)
	{
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
	if (moveStr[1] == 'x')
	{
		isCapture = true;
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
		}
		toFile = moveStr[3] - 'a';
		toRank = moveStr[4] - '1';
		moveLen = 5;
	}
	else if (moveStr[3] == 'x')
	{
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
			}

			if (toFile != -1)
			{
				fromFile = file1;
				toRank = moveStr[3] - '1';
				moveLen = 4;
			}
			else
			{
				if (moveStr[3] >= 'a' && moveStr[3] <= 'h')
				{
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
			toFile = moveStr[2] - 'a';
			toRank = moveStr[3] - '1';
			moveLen = 4;
		}
	}
search_moves:
	int toSquare = toFile | (toRank << 3);
	std::cout << "From File: " << fromFile << std::endl;
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
	
	std::cout << std::endl;;
	
	return {nullptr, moveStr + moveLen};
}


}
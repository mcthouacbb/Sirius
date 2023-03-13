#include "movegen.h"
#include "attacks.h"

BitBoard calcCheckBB(const Board& board, Color color)
{
	BitBoard nonKingBB = board.getAllPieces() ^ board.getPiece(PieceType::KING, color);

	Color oppColor = getOppColor(color);
	BitBoard kingBB = board.getPiece(PieceType::KING, oppColor);

	BitBoard checkBB = getKingAttacks(getLSB(kingBB));

	BitBoard queenBB = board.getPiece(PieceType::QUEEN, oppColor);
	while (queenBB)
	{
		uint32_t lsb = popLSB(queenBB);
		checkBB |= getQueenAttacks(lsb, nonKingBB);
	}

	BitBoard rookBB = board.getPiece(PieceType::ROOK, oppColor);
	while (rookBB)
	{
		uint32_t lsb = popLSB(rookBB);
		checkBB |= getRookAttacks(lsb, nonKingBB);
	}

	BitBoard bishopBB = board.getPiece(PieceType::BISHOP, oppColor);
	while (bishopBB)
	{
		uint32_t lsb = popLSB(bishopBB);
		checkBB |= getBishopAttacks(lsb, nonKingBB);
	}

	BitBoard knightBB = board.getPiece(PieceType::KNIGHT, oppColor);
	while (knightBB)
	{
		uint32_t lsb = popLSB(knightBB);
		checkBB |= getKnightAttacks(lsb);
	}

	BitBoard pawnBB = board.getPiece(PieceType::PAWN, oppColor);
	BitBoard pawnAttacks = shiftEast(pawnBB) | shiftWest(pawnBB);
	checkBB |= oppColor == Color::WHITE ? shiftNorth(pawnAttacks) : shiftSouth(pawnAttacks);
	return checkBB;
}

struct CheckPinData
{
	BitBoard checkingPieces;
	BitBoard pinnedPieces;
	BitBoard blockMask;
	BitBoard captureMask;
};

CheckPinData calcChecksPins(const Board& board, Color color)
{
	BitBoard kingBB = board.getPiece(PieceType::KING, color);
	if (kingBB == 0)
	{
	#if defined(_MSC_VER)
		__debugbreak();
	#elif defined(__GNUC__)
		throw std::runtime_error("King bitboard is 0");
	#endif
	}
	uint32_t kingIdx = getLSB(kingBB);

	Color oppColor = getOppColor(color);
	BitBoard checkingPieces = getKnightAttacks(kingIdx) & board.getPiece(PieceType::KNIGHT, oppColor);
	BitBoard pawnSquares = shiftEast(kingBB) | shiftWest(kingBB);
	pawnSquares = oppColor == Color::WHITE ? shiftSouth(pawnSquares) : shiftNorth(pawnSquares);
	checkingPieces |= pawnSquares & board.getPiece(PieceType::PAWN, oppColor);

	BitBoard captureMask = checkingPieces == 0 ? UINT64_MAX : checkingPieces;
	BitBoard blockMask = checkingPieces == 0 ? UINT64_MAX : 0;

	BitBoard oppQueens = board.getPiece(PieceType::QUEEN, oppColor);
	BitBoard oppRooks = board.getPiece(PieceType::ROOK, oppColor);
	BitBoard oppBishops = board.getPiece(PieceType::BISHOP, oppColor);

	BitBoard diagAttackers = oppQueens | oppBishops;
	BitBoard straightAttackers = oppQueens | oppRooks;

	BitBoard pinnedPieces = 0;

	BitBoard ray = getRay(kingIdx, Direction::NORTH);
	BitBoard rayAttackers = ray & straightAttackers;
	if (rayAttackers)
	{
		uint32_t closest = getLSB(rayAttackers);
		BitBoard closestBB = 1ull << closest;
		BitBoard inBetweenRay = ray ^ getRay(closest, Direction::NORTH) ^ closestBB;
		BitBoard inBetween = inBetweenRay & board.getAllPieces();
		if (inBetween == 0)
		{
			checkingPieces |= closestBB;
			captureMask &= closestBB;
			blockMask &= inBetweenRay;
		}
		else if ((inBetween & (inBetween - 1)) == 0)
		{
			pinnedPieces |= inBetween;
		}
	}

	ray = getRay(kingIdx, Direction::SOUTH);
	rayAttackers = ray & straightAttackers;
	if (rayAttackers)
	{
		uint32_t closest = getMSB(rayAttackers);
		BitBoard closestBB = 1ull << closest;
		BitBoard inBetweenRay = ray ^ getRay(closest, Direction::SOUTH) ^ closestBB;
		BitBoard inBetween = inBetweenRay & board.getAllPieces();
		if (inBetween == 0)
		{
			checkingPieces |= closestBB;
			captureMask &= closestBB;
			blockMask &= inBetweenRay;
		}
		else if ((inBetween & (inBetween - 1)) == 0)
		{
			pinnedPieces |= inBetween;
		}
	}

	ray = getRay(kingIdx, Direction::EAST);
	rayAttackers = ray & straightAttackers;
	if (rayAttackers)
	{
		uint32_t closest = getLSB(rayAttackers);
		BitBoard closestBB = 1ull << closest;
		BitBoard inBetweenRay = ray ^ getRay(closest, Direction::EAST) ^ closestBB;
		BitBoard inBetween = inBetweenRay & board.getAllPieces();
		if (inBetween == 0)
		{
			checkingPieces |= closestBB;
			captureMask &= closestBB;
			blockMask &= inBetweenRay;
		}
		else if ((inBetween & (inBetween - 1)) == 0)
		{
			pinnedPieces |= inBetween;
		}
	}

	ray = getRay(kingIdx, Direction::WEST);
	rayAttackers = ray & straightAttackers;
	if (rayAttackers)
	{
		uint32_t closest = getMSB(rayAttackers);
		BitBoard closestBB = 1ull << closest;
		BitBoard inBetweenRay = ray ^ getRay(closest, Direction::WEST) ^ closestBB;
		BitBoard inBetween = inBetweenRay & board.getAllPieces();
		if (inBetween == 0)
		{
			checkingPieces |= closestBB;
			captureMask &= closestBB;
			blockMask &= inBetweenRay;
		}
		else if ((inBetween & (inBetween - 1)) == 0)
		{
			pinnedPieces |= inBetween;
		}
	}

	ray = getRay(kingIdx, Direction::NORTH_EAST);
	rayAttackers = ray & diagAttackers;
	if (rayAttackers)
	{
		uint32_t closest = getLSB(rayAttackers);
		BitBoard closestBB = 1ull << closest;
		BitBoard inBetweenRay = ray ^ getRay(closest, Direction::NORTH_EAST) ^ closestBB;
		BitBoard inBetween = inBetweenRay & board.getAllPieces();
		if (inBetween == 0)
		{
			checkingPieces |= closestBB;
			captureMask &= closestBB;
			blockMask &= inBetweenRay;
		}
		else if ((inBetween & (inBetween - 1)) == 0)
		{
			pinnedPieces |= inBetween;
		}
	}

	ray = getRay(kingIdx, Direction::NORTH_WEST);
	rayAttackers = ray & diagAttackers;
	if (rayAttackers)
	{
		uint32_t closest = getLSB(rayAttackers);
		BitBoard closestBB = 1ull << closest;
		BitBoard inBetweenRay = ray ^ getRay(closest, Direction::NORTH_WEST) ^ closestBB;
		BitBoard inBetween = inBetweenRay & board.getAllPieces();
		if (inBetween == 0)
		{
			checkingPieces |= closestBB;
			captureMask &= closestBB;
			blockMask &= inBetweenRay;
		}
		else if ((inBetween & (inBetween - 1)) == 0)
		{
			pinnedPieces |= inBetween;
		}
	}

	ray = getRay(kingIdx, Direction::SOUTH_EAST);
	rayAttackers = ray & diagAttackers;
	if (rayAttackers)
	{
		uint32_t closest = getMSB(rayAttackers);
		BitBoard closestBB = 1ull << closest;
		BitBoard inBetweenRay = ray ^ getRay(closest, Direction::SOUTH_EAST) ^ closestBB;
		BitBoard inBetween = inBetweenRay & board.getAllPieces();
		if (inBetween == 0)
		{
			checkingPieces |= closestBB;
			captureMask &= closestBB;
			blockMask &= inBetweenRay;
		}
		else if ((inBetween & (inBetween - 1)) == 0)
		{
			pinnedPieces |= inBetween;
		}
	}

	ray = getRay(kingIdx, Direction::SOUTH_WEST);
	rayAttackers = ray & diagAttackers;
	if (rayAttackers)
	{
		uint32_t closest = getMSB(rayAttackers);
		BitBoard closestBB = 1ull << closest;
		BitBoard inBetweenRay = ray ^ getRay(closest, Direction::SOUTH_WEST) ^ closestBB;
		BitBoard inBetween = inBetweenRay & board.getAllPieces();
		if (inBetween == 0)
		{
			checkingPieces |= closestBB;
			captureMask &= closestBB;
			blockMask &= inBetweenRay;
		}
		else if ((inBetween & (inBetween - 1)) == 0)
		{
			pinnedPieces |= inBetween;
		}
	}

	return {
		checkingPieces,
		pinnedPieces,
		blockMask,
		captureMask
	};
}

bool canTakeEP(BitBoard eastRay, BitBoard westRay, BitBoard kingBB, BitBoard enemyAttackers, BitBoard allPieces)
{
	BitBoard eastBlockers = eastRay & allPieces;
	if (eastBlockers == 0)
		return true;

	BitBoard westBlockers = westRay & allPieces;
	if (westBlockers == 0)
		return true;

	// uint32_t closestEastBlocker = getLSB(eastBlockers);
	BitBoard closestEastBlocker = eastBlockers & -eastBlockers;
	BitBoard other;
	if (/*(1ull << closestEastBlocker)*/closestEastBlocker & kingBB)
	{
		other = enemyAttackers;
	}
	else if (/*(1ull << closestEastBlocker)*/closestEastBlocker & enemyAttackers)
	{
		other = kingBB;
	}
	else
	{
		return true;
	}

	uint32_t closestWestBlocker = getMSB(westBlockers);
	if ((1ull << closestWestBlocker) & other)
		return false;
	return true;
}

template<MoveGenType type, typename ...Args>
inline void addMove(Move*& moves, MoveType moveType, Args&& ...args)
{
	if constexpr (type == MoveGenType::LEGAL)
	{
		*moves++ = Move(moveType, std::forward<Args>(args)...);
	}
	else if constexpr (type == MoveGenType::CAPTURES)
	{
		if (moveType == MoveType::CAPTURE || moveType == MoveType::CAPTURE_PROMOTION || moveType == MoveType::ENPASSANT)
			*moves++ = Move(moveType, std::forward<Args>(args)...);
	}
}

template<MoveGenType type>
Move* genMoves(Board& board, Move* moves)
{
	BitBoard checkBB = calcCheckBB(board, board.getCurrPlayer());
	auto [checkingPieces, pinnedPieces, blockMask, captureMask] = calcChecksPins(board, board.getCurrPlayer());
	board.moveGenInfo().checkers = checkingPieces;
	if (board.getCurrPlayer() == Color::WHITE)
	{
		return genMovesWhite<type>(board, moves, checkBB, checkingPieces, pinnedPieces, blockMask, captureMask);
	}
	else
	{
		return genMovesBlack<type>(board, moves, checkBB, checkingPieces, pinnedPieces, blockMask, captureMask);
	}
}


template Move* genMoves<MoveGenType::LEGAL>(Board& board, Move* moves);
template Move* genMoves<MoveGenType::CAPTURES>(Board& board, Move* moves);

template<MoveGenType type>
void genWhiteKingMoves(const Board& board, Move*& moves, BitBoard checkBB);

template<MoveGenType type>
void genWhiteQueenMoves(const Board& board, Move*& moves, BitBoard pinnedPieces, BitBoard moveMask);

template<MoveGenType type>
void genWhiteRookMoves(const Board& board, Move*& moves, BitBoard pinnedPieces, BitBoard moveMask);

template<MoveGenType type>
void genWhiteBishopMoves(const Board& board, Move*& moves, BitBoard pinnedPieces, BitBoard moveMask);

template<MoveGenType type>
void genWhiteKnightMoves(const Board& board, Move*& moves, BitBoard pinnedPieces, BitBoard moveMask);

template<MoveGenType type>
void genWhitePawnMoves(const Board& board, Move*& moves, BitBoard pinnedPieces, BitBoard captureMask, BitBoard blockMask);

template<MoveGenType type>
Move* genMovesWhite(
	const Board& board,
	Move* moves,
	BitBoard checkBB,
	BitBoard checkingPieces,
	BitBoard pinnedPieces,
	BitBoard blockMask,
	BitBoard captureMask)
{
	if ((checkingPieces & (checkingPieces - 1)) == 0)
	{
		// not double check
		BitBoard moveMask = captureMask | blockMask;
		genWhiteKingMoves<type>(board, moves, checkBB);
		genWhiteKnightMoves<type>(board, moves, pinnedPieces, moveMask);
		genWhiteRookMoves<type>(board, moves, pinnedPieces, moveMask);
		genWhiteBishopMoves<type>(board, moves, pinnedPieces, moveMask);
		genWhiteQueenMoves<type>(board, moves, pinnedPieces, moveMask);
		genWhitePawnMoves<type>(board, moves, pinnedPieces, captureMask, blockMask);
	}
	else
	{
		// double check, only king moves
		genWhiteKingMoves<type>(board, moves, checkBB);
	}
	return moves;
}

template<MoveGenType type>
void genWhiteKingMoves(const Board& board, Move*& moves, BitBoard checkBB)
{
	BitBoard kingBB = board.getPiece(PieceType::KING, Color::WHITE);
	uint32_t kingIdx = getLSB(kingBB);

	BitBoard kingAttacks = getKingAttacks(kingIdx);
	BitBoard attackBB = kingAttacks & ~board.getColor(Color::WHITE);
	attackBB &= ~checkBB;

	while (attackBB)
	{
		uint32_t lsb = popLSB(attackBB);
		PieceType capture = board.getBlackPieceAt(lsb);
		// if (capture == PieceType::NONE)
			// *moves++ = Move(MoveType::NONE, kingIdx, lsb, PieceType::KING));
		// else
			// *moves++ = Move(MoveType::CAPTURE, kingIdx, lsb, PieceType::KING, capture));
		MoveType moveType = capture == PieceType::NONE ? MoveType::NONE : MoveType::CAPTURE;
		addMove<type>(moves, moveType, kingIdx, lsb, PieceType::KING, capture);
		if (capture != PieceType::NONE)
		{
			addMove<type>(moves, MoveType::CAPTURE, kingIdx, lsb, PieceType::KING, capture);
		}
	}

	if (board.getCastlingRights(Color::WHITE, CastleSide::KING) && (checkBB & WKSC_CHECK_SQUARES) == 0 && (WKSC_BLOCK_SQUARES & board.getAllPieces()) == 0)
	{
		addMove<type>(moves, MoveType::KSIDE_CASTLE, kingIdx, kingIdx + 2, PieceType::KING);
	}

	if (board.getCastlingRights(Color::WHITE, CastleSide::QUEEN) && (checkBB & WQSC_CHECK_SQUARES) == 0 && (WQSC_BLOCK_SQUARES & board.getAllPieces()) == 0)
	{
		addMove<type>(moves, MoveType::QSIDE_CASTLE, kingIdx, kingIdx - 2, PieceType::KING);
	}
}

template<MoveGenType type>
void genWhiteQueenMoves(const Board& board, Move*& moves, BitBoard pinnedPieces, BitBoard moveMask)
{
	BitBoard queenBB = board.getPiece(PieceType::QUEEN, Color::WHITE);
	BitBoard kingBB = board.getPiece(PieceType::KING, Color::WHITE);
	uint32_t kingIdx = getLSB(kingBB);
	uint32_t kingX = kingIdx & 7, kingY = kingIdx >> 3;

	while (queenBB)
	{
		uint32_t queenIdx = popLSB(queenBB);

		BitBoard queenAttacks;
		if ((1ull << queenIdx) & pinnedPieces)
		{
			uint32_t queenX = queenIdx & 7, queenY = queenIdx >> 3;
			if (queenX == kingX)
			{
				queenAttacks = getVerticalAttacks(queenIdx, board.getAllPieces());
			}
			else if (queenY == kingY)
			{
				queenAttacks = getHorizontalAttacks(queenIdx, board.getAllPieces());
			}
			else
			{
				bool isDiagonal = (queenX > kingX) == (queenY > kingY);
				if (isDiagonal)
				{
					queenAttacks = getDiagonalAttacks(queenIdx, board.getAllPieces());
				}
				else
				{
					queenAttacks = getAntidiagonalAttacks(queenIdx, board.getAllPieces());
				}
			}
		}
		else
		{
			queenAttacks = getQueenAttacks(queenIdx, board.getAllPieces());
		}

		BitBoard attackBB = queenAttacks & ~board.getColor(Color::WHITE);
		attackBB &= moveMask;

		while (attackBB)
		{
			uint32_t lsb = popLSB(attackBB);
			PieceType capture = board.getBlackPieceAt(lsb);
			MoveType moveType = capture == PieceType::NONE ? MoveType::NONE : MoveType::CAPTURE;
			addMove<type>(moves, moveType, queenIdx, lsb, PieceType::QUEEN, capture);
		}
	}
}

template<MoveGenType type>
void genWhiteRookMoves(const Board& board, Move*& moves, BitBoard pinnedPieces, BitBoard moveMask)
{
	BitBoard rookBB = board.getPiece(PieceType::ROOK, Color::WHITE);
	BitBoard kingBB = board.getPiece(PieceType::KING, Color::WHITE);
	uint32_t kingIdx = getLSB(kingBB);
	uint32_t kingX = kingIdx & 7, kingY = kingIdx >> 3;

	while (rookBB)
	{
		uint32_t rookIdx = popLSB(rookBB);
		BitBoard rookAttacks;
		if ((1ull << rookIdx) & pinnedPieces)
		{
			uint32_t rookX = rookIdx & 7, rookY = rookIdx >> 3;
			if (kingX != rookX && kingY != rookY)
				continue;

			if (kingX == rookX)
			{
				rookAttacks = getVerticalAttacks(rookIdx, board.getAllPieces());
			}
			else
			{
				rookAttacks = getHorizontalAttacks(rookIdx, board.getAllPieces());
			}
		}
		else
		{
			rookAttacks = getRookAttacks(rookIdx, board.getAllPieces());
		}

		BitBoard attackBB = rookAttacks & ~board.getColor(Color::WHITE);
		attackBB &= moveMask;

		while (attackBB)
		{
			uint32_t lsb = popLSB(attackBB);
			PieceType capture = board.getBlackPieceAt(lsb);
			MoveType moveType = capture == PieceType::NONE ? MoveType::NONE : MoveType::CAPTURE;
			addMove<type>(moves, moveType, rookIdx, lsb, PieceType::ROOK, capture);
		}
	}
}

template<MoveGenType type>
void genWhiteBishopMoves(const Board& board, Move*& moves, BitBoard pinnedPieces, BitBoard moveMask)
{
	BitBoard bishopBB = board.getPiece(PieceType::BISHOP, Color::WHITE);
	BitBoard kingBB = board.getPiece(PieceType::KING, Color::WHITE);
	uint32_t kingIdx = getLSB(kingBB);
	uint32_t kingX = kingIdx & 7, kingY = kingIdx >> 3;
	while (bishopBB)
	{
		uint32_t bishopIdx = popLSB(bishopBB);
		BitBoard bishopAttacks;
		if ((1ull << bishopIdx) & pinnedPieces)
		{
			uint32_t bishopX = bishopIdx & 7, bishopY = bishopIdx >> 3;
			if (bishopX == kingX || bishopY == kingY)
				continue;

			bool isDiagonal = (bishopX > kingX) == (bishopY > kingY);
			if (isDiagonal)
			{
				bishopAttacks = getDiagonalAttacks(bishopIdx, board.getAllPieces());
			}
			else
			{
				bishopAttacks = getAntidiagonalAttacks(bishopIdx, board.getAllPieces());
			}
		}
		else
		{
			bishopAttacks = getBishopAttacks(bishopIdx, board.getAllPieces());
		}

		BitBoard attackBB = bishopAttacks & ~board.getColor(Color::WHITE);
		attackBB &= moveMask;

		while (attackBB)
		{
			uint32_t lsb = popLSB(attackBB);
			PieceType capture = board.getBlackPieceAt(lsb);
			MoveType moveType = capture == PieceType::NONE ? MoveType::NONE : MoveType::CAPTURE;
			addMove<type>(moves, moveType, bishopIdx, lsb, PieceType::BISHOP, capture);
		}
	}
}

template<MoveGenType type>
void genWhiteKnightMoves(const Board& board, Move*& moves, BitBoard pinnedPieces, BitBoard moveMask)
{
	BitBoard knightBB = board.getPiece(PieceType::KNIGHT, Color::WHITE);
	knightBB &= ~pinnedPieces;

	while (knightBB)
	{
		uint32_t knightIdx = popLSB(knightBB);
		BitBoard knightAttacks = getKnightAttacks(knightIdx);
		BitBoard attackBB = knightAttacks & ~board.getColor(Color::WHITE);
		attackBB &= moveMask;

		while (attackBB)
		{
			uint32_t lsb = popLSB(attackBB);
			PieceType capture = board.getBlackPieceAt(lsb);
			MoveType moveType = capture == PieceType::NONE ? MoveType::NONE : MoveType::CAPTURE;
			addMove<type>(moves, moveType, knightIdx, lsb, PieceType::KNIGHT, capture);
		}
	}
}

template<MoveGenType type>
void genWhitePawnMoves(const Board& board, Move*& moves, BitBoard pinnedPieces, BitBoard captureMask, BitBoard blockMask)
{
	BitBoard pawnBB = board.getPiece(PieceType::PAWN, Color::WHITE);
	BitBoard kingBB = board.getPiece(PieceType::KING, Color::WHITE);
	uint32_t kingIdx = getLSB(kingBB);
	uint32_t kingX = kingIdx & 7, kingY = kingIdx >> 3;
	BitBoard straightAttackers = board.getPiece(PieceType::ROOK, Color::BLACK) | board.getPiece(PieceType::QUEEN, Color::BLACK);
	
	while (pawnBB)
	{
		BitBoard pawnBlockMask = blockMask;
		BitBoard pawnCaptureMask = captureMask;
		uint32_t pawnIdx = popLSB(pawnBB);
		if ((1ull << pawnIdx) & pinnedPieces)
		{
			uint32_t pawnX = pawnIdx & 7, pawnY = pawnIdx >> 3;
			if (pawnX == kingX)
			{
				pawnCaptureMask = 0;
			}
			else if (pawnY <= kingY)
			{
				continue;
			}
			else
			{
				if (pawnX > kingX)
				{
					pawnCaptureMask &= 1ull << (pawnIdx + 9);
					pawnBlockMask = 0;
				}
				else
				{
					pawnCaptureMask &= 1ull << (pawnIdx + 7);
					pawnBlockMask = 0;
				}
			}
		}

		if (pawnIdx > 47)
		{
			// WILL PROMOTE NEXT MOVE
			BitBoard promotionBB = 1ull << (pawnIdx + 8);
			if ((board.getAllPieces() & promotionBB) == 0 && (promotionBB & pawnBlockMask))
			{
				addMove<type>(moves, MoveType::PROMOTION, pawnIdx, pawnIdx + 8, PieceType::PAWN, PieceType::QUEEN);
				addMove<type>(moves, MoveType::PROMOTION, pawnIdx, pawnIdx + 8, PieceType::PAWN, PieceType::ROOK);
				addMove<type>(moves, MoveType::PROMOTION, pawnIdx, pawnIdx + 8, PieceType::PAWN, PieceType::BISHOP);
				addMove<type>(moves, MoveType::PROMOTION, pawnIdx, pawnIdx + 8, PieceType::PAWN, PieceType::KNIGHT);
			}

			BitBoard leftCaptureBB = shiftWest(promotionBB);
			if (leftCaptureBB & pawnCaptureMask)
			{
				PieceType capture = board.getBlackPieceFrom(leftCaptureBB);
				if (capture != PieceType::NONE)
				{
					addMove<type>(moves, MoveType::CAPTURE_PROMOTION, pawnIdx, pawnIdx + 7, capture, PieceType::QUEEN);
					addMove<type>(moves, MoveType::CAPTURE_PROMOTION, pawnIdx, pawnIdx + 7, capture, PieceType::ROOK);
					addMove<type>(moves, MoveType::CAPTURE_PROMOTION, pawnIdx, pawnIdx + 7, capture, PieceType::BISHOP);
					addMove<type>(moves, MoveType::CAPTURE_PROMOTION, pawnIdx, pawnIdx + 7, capture, PieceType::KNIGHT);
				}
			}

			BitBoard rightCaptureBB = shiftEast(promotionBB);
			if (rightCaptureBB & pawnCaptureMask)
			{
				PieceType capture = board.getBlackPieceFrom(rightCaptureBB);
				if (capture != PieceType::NONE)
				{
					addMove<type>(moves, MoveType::CAPTURE_PROMOTION, pawnIdx, pawnIdx + 9, capture, PieceType::QUEEN);
					addMove<type>(moves, MoveType::CAPTURE_PROMOTION, pawnIdx, pawnIdx + 9, capture, PieceType::ROOK);
					addMove<type>(moves, MoveType::CAPTURE_PROMOTION, pawnIdx, pawnIdx + 9, capture, PieceType::BISHOP);
					addMove<type>(moves, MoveType::CAPTURE_PROMOTION, pawnIdx, pawnIdx + 9, capture, PieceType::KNIGHT);
				}
			}
		}
		else
		{
			BitBoard pushBB = 1ull << (pawnIdx + 8);
			if ((board.getAllPieces() & pushBB) == 0)
			{
				if (pushBB & pawnBlockMask)
				{
					addMove<type>(moves, MoveType::NONE, pawnIdx, pawnIdx + 8, PieceType::PAWN);
				}
				BitBoard doublePushBB = shiftNorth(pushBB);
				if (pawnIdx < 16 && (board.getAllPieces() & doublePushBB) == 0 && (doublePushBB & pawnBlockMask))
				{
					addMove<type>(moves, MoveType::DOUBLE_PUSH, pawnIdx, pawnIdx + 16, PieceType::PAWN);
				}
			}

			BitBoard leftCaptureBB = shiftWest(pushBB);
			if (leftCaptureBB)
			{
				if (leftCaptureBB & pawnCaptureMask)
				{
					PieceType capture = board.getBlackPieceFrom(leftCaptureBB);
					if (capture != PieceType::NONE)
					{
						addMove<type>(moves, MoveType::CAPTURE, pawnIdx, pawnIdx + 7, PieceType::PAWN, capture);
						goto no_left_ep;
					}
				}
				if ((leftCaptureBB & board.getEnpassant()) && (shiftSouth(board.getEnpassant()) & pawnCaptureMask))
				{
					BitBoard westRay = getRay(pawnIdx - 1, Direction::WEST);
					BitBoard eastRay = getRay(pawnIdx, Direction::EAST);
					if (canTakeEP(eastRay, westRay, kingBB, straightAttackers, board.getAllPieces()))
					{
						addMove<type>(moves, MoveType::ENPASSANT, pawnIdx, pawnIdx + 7, PieceType::PAWN);
					}
					//BitBoard rays = getRay(pawnIdx, Direction::EAST) | getRay(pawnIdx - 1, Direction::WEST);
					//if ((rays & straightAttackers) == 0 || (rays & kingBB) == 0)
					//{
					// }
				}
			}
		no_left_ep:
			BitBoard rightCaptureBB = shiftEast(pushBB);
			if (rightCaptureBB)
			{
				if (rightCaptureBB & pawnCaptureMask)
				{
					PieceType capture = board.getBlackPieceFrom(rightCaptureBB);
					if (capture != PieceType::NONE)
					{
						addMove<type>(moves, MoveType::CAPTURE, pawnIdx, pawnIdx + 9, PieceType::PAWN, capture);
						goto no_right_ep;
					}
				}

				if ((rightCaptureBB & board.getEnpassant()) && (shiftSouth(board.getEnpassant()) & pawnCaptureMask))
				{
					BitBoard eastRay = getRay(pawnIdx + 1, Direction::EAST);
					BitBoard westRay = getRay(pawnIdx, Direction::WEST);
					if (canTakeEP(eastRay, westRay, kingBB, straightAttackers, board.getAllPieces()))
					{
						addMove<type>(moves, MoveType::ENPASSANT, pawnIdx, pawnIdx + 9, PieceType::PAWN);
					}
					//BitBoard rays = getRay(pawnIdx, Direction::WEST) | getRay(pawnIdx + 1, Direction::EAST);
					//if ((rays & straightAttackers) == 0 || (rays & kingBB) == 0)
					//{
						//*moves++ = Move(MoveType::ENPASSANT, pawnIdx, pawnIdx + 9, PieceType::PAWN));
					//}
				}
			}
		no_right_ep:
			;
		}
	}
}


template<MoveGenType type>
void genBlackKingMoves(const Board& board, Move*& moves, BitBoard checkBB);

template<MoveGenType type>
void genBlackQueenMoves(const Board& board, Move*& moves, BitBoard pinnedPieces, BitBoard moveMask);

template<MoveGenType type>
void genBlackRookMoves(const Board& board, Move*& moves, BitBoard pinnedPieces, BitBoard moveMask);

template<MoveGenType type>
void genBlackBishopMoves(const Board& board, Move*& moves, BitBoard pinnedPieces, BitBoard moveMask);

template<MoveGenType type>
void genBlackKnightMoves(const Board& board, Move*& moves, BitBoard pinnedPieces, BitBoard moveMask);

template<MoveGenType type>
void genBlackPawnMoves(const Board& board, Move*& moves, BitBoard pinnedPieces, BitBoard captureMask, BitBoard blockMask);

template<MoveGenType type>
Move* genMovesBlack(
	const Board& board,
	Move* moves,
	BitBoard checkBB,
	BitBoard checkingPieces,
	BitBoard pinnedPieces,
	BitBoard blockMask,
	BitBoard captureMask)
{
	if ((checkingPieces & (checkingPieces - 1)) == 0)
	{
		// not double check
		BitBoard moveMask = captureMask | blockMask;
		genBlackKingMoves<type>(board, moves, checkBB);
		genBlackKnightMoves<type>(board, moves, pinnedPieces, moveMask);
		genBlackRookMoves<type>(board, moves, pinnedPieces, moveMask);
		genBlackBishopMoves<type>(board, moves, pinnedPieces, moveMask);
		genBlackQueenMoves<type>(board, moves, pinnedPieces, moveMask);
		genBlackPawnMoves<type>(board, moves, pinnedPieces, captureMask, blockMask);
	}
	else
	{
		// double check, only king moves
		genBlackKingMoves<type>(board, moves, checkBB);
	}
	return moves;
}


template<MoveGenType type>
void genBlackKingMoves(const Board& board, Move*& moves, BitBoard checkBB)
{
	BitBoard kingBB = board.getPiece(PieceType::KING, Color::BLACK);
	uint32_t kingIdx = getLSB(kingBB);

	BitBoard kingAttacks = getKingAttacks(kingIdx);
	BitBoard attackBB = kingAttacks & ~board.getColor(Color::BLACK);
	attackBB &= ~checkBB;

	while (attackBB)
	{
		uint32_t lsb = popLSB(attackBB);
		PieceType capture = board.getWhitePieceAt(lsb);
		// if (capture == PieceType::NONE)
			// *moves++ = Move(MoveType::NONE, kingIdx, lsb, PieceType::KING));
		// else
			// *moves++ = Move(MoveType::CAPTURE, kingIdx, lsb, PieceType::KING, capture));
		MoveType moveType = capture == PieceType::NONE ? MoveType::NONE : MoveType::CAPTURE;
		addMove<type>(moves, moveType, kingIdx, lsb, PieceType::KING, capture);
	}

	if (board.getCastlingRights(Color::BLACK, CastleSide::KING) && (checkBB & BKSC_CHECK_SQUARES) == 0 && (BKSC_BLOCK_SQUARES & board.getAllPieces()) == 0)
	{
		addMove<type>(moves, MoveType::KSIDE_CASTLE, kingIdx, kingIdx + 2, PieceType::KING);
	}

	if (board.getCastlingRights(Color::BLACK, CastleSide::QUEEN) && (checkBB & BQSC_CHECK_SQUARES) == 0 && (BQSC_BLOCK_SQUARES & board.getAllPieces()) == 0)
	{
		addMove<type>(moves, MoveType::QSIDE_CASTLE, kingIdx, kingIdx - 2, PieceType::KING);
	}
}

template<MoveGenType type>
void genBlackQueenMoves(const Board& board, Move*& moves, BitBoard pinnedPieces, BitBoard moveMask)
{
	BitBoard queenBB = board.getPiece(PieceType::QUEEN, Color::BLACK);
	BitBoard kingBB = board.getPiece(PieceType::KING, Color::BLACK);
	uint32_t kingIdx = getLSB(kingBB);
	uint32_t kingX = kingIdx & 7, kingY = kingIdx >> 3;

	while (queenBB)
	{
		uint32_t queenIdx = popLSB(queenBB);

		BitBoard queenAttacks;
		if ((1ull << queenIdx) & pinnedPieces)
		{
			uint32_t queenX = queenIdx & 7, queenY = queenIdx >> 3;
			if (queenX == kingX)
			{
				queenAttacks = getVerticalAttacks(queenIdx, board.getAllPieces());
			}
			else if (queenY == kingY)
			{
				queenAttacks = getHorizontalAttacks(queenIdx, board.getAllPieces());
			}
			else
			{
				bool isDiagonal = (queenX > kingX) == (queenY > kingY);
				if (isDiagonal)
				{
					queenAttacks = getDiagonalAttacks(queenIdx, board.getAllPieces());
				}
				else
				{
					queenAttacks = getAntidiagonalAttacks(queenIdx, board.getAllPieces());
				}
			}
		}
		else
		{
			queenAttacks = getQueenAttacks(queenIdx, board.getAllPieces());
		}

		BitBoard attackBB = queenAttacks & ~board.getColor(Color::BLACK);
		attackBB &= moveMask;

		while (attackBB)
		{
			uint32_t lsb = popLSB(attackBB);
			PieceType capture = board.getWhitePieceAt(lsb);
			MoveType moveType = capture == PieceType::NONE ? MoveType::NONE : MoveType::CAPTURE;
			addMove<type>(moves, moveType, queenIdx, lsb, PieceType::QUEEN, capture);
		}
	}
}

template<MoveGenType type>
void genBlackRookMoves(const Board& board, Move*& moves, BitBoard pinnedPieces, BitBoard moveMask)
{
	BitBoard rookBB = board.getPiece(PieceType::ROOK, Color::BLACK);
	BitBoard kingBB = board.getPiece(PieceType::KING, Color::BLACK);
	uint32_t kingIdx = getLSB(kingBB);
	uint32_t kingX = kingIdx & 7, kingY = kingIdx >> 3;

	while (rookBB)
	{
		uint32_t rookIdx = popLSB(rookBB);
		BitBoard rookAttacks;
		if ((1ull << rookIdx) & pinnedPieces)
		{
			uint32_t rookX = rookIdx & 7, rookY = rookIdx >> 3;
			if (kingX != rookX && kingY != rookY)
				continue;

			if (kingX == rookX)
			{
				rookAttacks = getVerticalAttacks(rookIdx, board.getAllPieces());
			}
			else
			{
				rookAttacks = getHorizontalAttacks(rookIdx, board.getAllPieces());
			}
		}
		else
		{
			rookAttacks = getRookAttacks(rookIdx, board.getAllPieces());
		}

		BitBoard attackBB = rookAttacks & ~board.getColor(Color::BLACK);
		attackBB &= moveMask;

		while (attackBB)
		{
			uint32_t lsb = popLSB(attackBB);
			PieceType capture = board.getWhitePieceAt(lsb);
			MoveType moveType = capture == PieceType::NONE ? MoveType::NONE : MoveType::CAPTURE;
			addMove<type>(moves, moveType, rookIdx, lsb, PieceType::ROOK, capture);
		}
	}
}

template<MoveGenType type>
void genBlackBishopMoves(const Board& board, Move*& moves, BitBoard pinnedPieces, BitBoard moveMask)
{
	BitBoard bishopBB = board.getPiece(PieceType::BISHOP, Color::BLACK);
	BitBoard kingBB = board.getPiece(PieceType::KING, Color::BLACK);
	uint32_t kingIdx = getLSB(kingBB);
	uint32_t kingX = kingIdx & 7, kingY = kingIdx >> 3;
	while (bishopBB)
	{
		uint32_t bishopIdx = popLSB(bishopBB);
		BitBoard bishopAttacks;
		if ((1ull << bishopIdx) & pinnedPieces)
		{
			uint32_t bishopX = bishopIdx & 7, bishopY = bishopIdx >> 3;
			if (bishopX == kingX || bishopY == kingY)
				continue;

			bool isDiagonal = (bishopX > kingX) == (bishopY > kingY);
			if (isDiagonal)
			{
				bishopAttacks = getDiagonalAttacks(bishopIdx, board.getAllPieces());
			}
			else
			{
				bishopAttacks = getAntidiagonalAttacks(bishopIdx, board.getAllPieces());
			}
		}
		else
		{
			bishopAttacks = getBishopAttacks(bishopIdx, board.getAllPieces());
		}

		BitBoard attackBB = bishopAttacks & ~board.getColor(Color::BLACK);
		attackBB &= moveMask;

		while (attackBB)
		{
			uint32_t lsb = popLSB(attackBB);
			PieceType capture = board.getWhitePieceAt(lsb);
			MoveType moveType = capture == PieceType::NONE ? MoveType::NONE : MoveType::CAPTURE;
			addMove<type>(moves, moveType, bishopIdx, lsb, PieceType::BISHOP, capture);
		}
	}
}

template<MoveGenType type>
void genBlackKnightMoves(const Board& board, Move*& moves, BitBoard pinnedPieces, BitBoard moveMask)
{
	BitBoard knightBB = board.getPiece(PieceType::KNIGHT, Color::BLACK);
	knightBB &= ~pinnedPieces;

	while (knightBB)
	{
		uint32_t knightIdx = popLSB(knightBB);
		BitBoard knightAttacks = getKnightAttacks(knightIdx);
		BitBoard attackBB = knightAttacks & ~board.getColor(Color::BLACK);
		attackBB &= moveMask;

		while (attackBB)
		{
			uint32_t lsb = popLSB(attackBB);
			PieceType capture = board.getWhitePieceAt(lsb);
			MoveType moveType = capture == PieceType::NONE ? MoveType::NONE : MoveType::CAPTURE;
			addMove<type>(moves, moveType, knightIdx, lsb, PieceType::KNIGHT, capture);
		}
	}
}

template<MoveGenType type>
void genBlackPawnMoves(const Board& board, Move*& moves, BitBoard pinnedPieces, BitBoard captureMask, BitBoard blockMask)
{
	BitBoard pawnBB = board.getPiece(PieceType::PAWN, Color::BLACK);
	BitBoard kingBB = board.getPiece(PieceType::KING, Color::BLACK);
	uint32_t kingIdx = getLSB(kingBB);
	uint32_t kingX = kingIdx & 7, kingY = kingIdx >> 3;
	BitBoard straightAttackers = board.getPiece(PieceType::ROOK, Color::WHITE) | board.getPiece(PieceType::QUEEN, Color::WHITE);

	while (pawnBB)
	{
		BitBoard pawnBlockMask = blockMask;
		BitBoard pawnCaptureMask = captureMask;
		uint32_t pawnIdx = popLSB(pawnBB);
		if ((1ull << pawnIdx) & pinnedPieces)
		{
			uint32_t pawnX = pawnIdx & 7, pawnY = pawnIdx >> 3;
			if (pawnX == kingX)
			{
				pawnCaptureMask = 0;
			}
			else if (pawnY >= kingY)
			{
				continue;
			}
			else
			{
				if (pawnX > kingX)
				{
					pawnCaptureMask &= 1ull << (pawnIdx - 7);
					pawnBlockMask = 0;
				}
				else
				{
					pawnCaptureMask &= 1ull << (pawnIdx - 9);
					pawnBlockMask = 0;
				}
			}
		}

		if (pawnIdx < 16)
		{
			// WILL PROMOTE NEXT MOVE
			BitBoard promotionBB = 1ull << (pawnIdx - 8);
			if ((board.getAllPieces() & promotionBB) == 0 && (promotionBB & pawnBlockMask))
			{
				addMove<type>(moves, MoveType::PROMOTION, pawnIdx, pawnIdx - 8, PieceType::PAWN, PieceType::QUEEN);
				addMove<type>(moves, MoveType::PROMOTION, pawnIdx, pawnIdx - 8, PieceType::PAWN, PieceType::ROOK);
				addMove<type>(moves, MoveType::PROMOTION, pawnIdx, pawnIdx - 8, PieceType::PAWN, PieceType::BISHOP);
				addMove<type>(moves, MoveType::PROMOTION, pawnIdx, pawnIdx - 8, PieceType::PAWN, PieceType::KNIGHT);
			}

			BitBoard leftCaptureBB = shiftWest(promotionBB);
			if (leftCaptureBB & pawnCaptureMask)
			{
				PieceType capture = board.getWhitePieceFrom(leftCaptureBB);
				if (capture != PieceType::NONE)
				{
					addMove<type>(moves, MoveType::CAPTURE_PROMOTION, pawnIdx, pawnIdx - 9, capture, PieceType::QUEEN);
					addMove<type>(moves, MoveType::CAPTURE_PROMOTION, pawnIdx, pawnIdx - 9, capture, PieceType::ROOK);
					addMove<type>(moves, MoveType::CAPTURE_PROMOTION, pawnIdx, pawnIdx - 9, capture, PieceType::BISHOP);
					addMove<type>(moves, MoveType::CAPTURE_PROMOTION, pawnIdx, pawnIdx - 9, capture, PieceType::KNIGHT);
				}
			}

			BitBoard rightCaptureBB = shiftEast(promotionBB);
			if (rightCaptureBB & pawnCaptureMask)
			{
				PieceType capture = board.getWhitePieceFrom(rightCaptureBB);
				if (capture != PieceType::NONE)
				{
					addMove<type>(moves, MoveType::CAPTURE_PROMOTION, pawnIdx, pawnIdx - 7, capture, PieceType::QUEEN);
					addMove<type>(moves, MoveType::CAPTURE_PROMOTION, pawnIdx, pawnIdx - 7, capture, PieceType::ROOK);
					addMove<type>(moves, MoveType::CAPTURE_PROMOTION, pawnIdx, pawnIdx - 7, capture, PieceType::BISHOP);
					addMove<type>(moves, MoveType::CAPTURE_PROMOTION, pawnIdx, pawnIdx - 7, capture, PieceType::KNIGHT);
				}
			}
		}
		else
		{
			BitBoard pushBB = 1ull << (pawnIdx - 8);
			if ((board.getAllPieces() & pushBB) == 0)
			{
				if (pushBB & pawnBlockMask)
				{
					addMove<type>(moves, MoveType::NONE, pawnIdx, pawnIdx - 8, PieceType::PAWN);
				}
				BitBoard doublePushBB = shiftSouth(pushBB);
				if (pawnIdx > 47 && (board.getAllPieces() & doublePushBB) == 0 && (doublePushBB & pawnBlockMask))
				{
					addMove<type>(moves, MoveType::DOUBLE_PUSH, pawnIdx, pawnIdx - 16, PieceType::PAWN);
				}
			}

			BitBoard leftCaptureBB = shiftWest(pushBB);
			if (leftCaptureBB)
			{
				if (leftCaptureBB & pawnCaptureMask)
				{
					PieceType capture = board.getWhitePieceFrom(leftCaptureBB);
					if (capture != PieceType::NONE)
					{
						addMove<type>(moves, MoveType::CAPTURE, pawnIdx, pawnIdx - 9, PieceType::PAWN, capture);
						goto no_left_ep;
					}
				}
				if ((leftCaptureBB & board.getEnpassant()) && (shiftNorth(board.getEnpassant()) & pawnCaptureMask))
				{
					BitBoard eastRay = getRay(pawnIdx, Direction::EAST);
					BitBoard westRay = getRay(pawnIdx - 1, Direction::WEST);
					if (canTakeEP(eastRay, westRay, kingBB, straightAttackers, board.getAllPieces()))
					{
						addMove<type>(moves, MoveType::ENPASSANT, pawnIdx, pawnIdx - 9, PieceType::PAWN);
					}
					//BitBoard rays = getRay(pawnIdx, Direction::EAST) | getRay(pawnIdx - 1, Direction::WEST);
					//if ((rays & straightAttackers) == 0 || (rays & kingBB) == 0)
					//{
					//	*moves++ = Move(MoveType::ENPASSANT, pawnIdx, pawnIdx - 9, PieceType::PAWN));
					//}
				}
			}
		no_left_ep:
			BitBoard rightCaptureBB = shiftEast(pushBB);
			if (rightCaptureBB)
			{
				if (rightCaptureBB & pawnCaptureMask)
				{
					PieceType capture = board.getWhitePieceFrom(rightCaptureBB);
					if (capture != PieceType::NONE)
					{
						addMove<type>(moves, MoveType::CAPTURE, pawnIdx, pawnIdx - 7, PieceType::PAWN, capture);
						goto no_right_ep;
					}
				}

				if ((rightCaptureBB & board.getEnpassant()) && (shiftNorth(board.getEnpassant()) & pawnCaptureMask))
				{
					BitBoard eastRay = getRay(pawnIdx + 1, Direction::EAST);
					BitBoard westRay = getRay(pawnIdx, Direction::WEST);
					if (canTakeEP(eastRay, westRay, kingBB, straightAttackers, board.getAllPieces()))
					{
						addMove<type>(moves, MoveType::ENPASSANT, pawnIdx, pawnIdx - 7, PieceType::PAWN);
					}
					//BitBoard rays = getRay(pawnIdx, Direction::WEST) | getRay(pawnIdx + 1, Direction::EAST);
					//if ((rays & straightAttackers) == 0 || (rays & kingBB) == 0)
					//{
					//	*moves++ = Move(MoveType::ENPASSANT, pawnIdx, pawnIdx - 7, PieceType::PAWN));
					//}
				}
			}
		no_right_ep:
			;
		}
	}
}
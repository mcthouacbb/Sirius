#include "movegen.h"
#include "attacks.h"

CheckInfo calcCheckInfo(const Board& board, Color color)
{
	BitBoard usBB = board.getColor(color);
	BitBoard oppBB = board.getColor(flip(color));
	BitBoard kingBB = board.getPieces(PieceType::KING);
	
	if ((kingBB & usBB) == 0)
	{
		throw std::runtime_error("King bb is zero");
	}

	if ((kingBB & oppBB) == 0)
	{
		throw std::runtime_error("Opp bb is zero");
	}
	
	uint32_t oppKingIdx = getLSB(kingBB & oppBB);

	// calculating illegal king squares
	BitBoard checkBB = attacks::getKingAttacks(oppKingIdx);

	BitBoard oppPawns = board.getPieces(PieceType::PAWN) & oppBB;
	if (color == Color::WHITE)
		checkBB |= attacks::getPawnBBAttacks<Color::BLACK>(oppPawns);
	else
		checkBB |= attacks::getPawnBBAttacks<Color::WHITE>(oppPawns);

	
	BitBoard oppKnights = board.getPieces(PieceType::KNIGHT) & oppBB;
	
	
	uint32_t kingIdx = getLSB(kingBB & usBB);
	
	BitBoard checkers = 0;
	checkers |= attacks::getPawnAttacks(color, kingIdx) & oppPawns;
	checkers |= attacks::getKnightAttacks(kingIdx) & oppKnights;
	
	while (oppKnights)
	{
		uint32_t knightIdx = popLSB(oppKnights);
		checkBB |= attacks::getKnightAttacks(knightIdx);
	}

	BitBoard checkBlockers = board.getAllPieces() ^ (usBB & kingBB);

	BitBoard oppBishops = board.getPieces(PieceType::BISHOP) & oppBB;
	BitBoard oppRooks = board.getPieces(PieceType::ROOK) & oppBB;
	BitBoard oppQueens = board.getPieces(PieceType::QUEEN) & oppBB;

	BitBoard diagPieces = oppQueens | oppBishops;
	BitBoard straightPieces = oppQueens | oppRooks;
	while (oppBishops)
	{
		uint32_t bishopIdx = popLSB(oppBishops);
		checkBB |= attacks::getBishopAttacks(bishopIdx, checkBlockers);
	}

	while (oppRooks)
	{
		uint32_t rookIdx = popLSB(oppRooks);
		checkBB |= attacks::getRookAttacks(rookIdx, checkBlockers);
	}

	while (oppQueens)
	{
		uint32_t queenIdx = popLSB(oppQueens);
		checkBB |= attacks::getQueenAttacks(queenIdx, checkBlockers);
	}

	
	// calculating checking pieces, pinned pieces, and valid move locations

	BitBoard moveMask = checkers == 0 ? 0xFFFFFFFFFFFFFFFFull : checkers;
	
	diagPieces &= attacks::getBishopAttacks(kingIdx, diagPieces);
	straightPieces &= attacks::getRookAttacks(kingIdx, straightPieces);

	BitBoard sliders = diagPieces | straightPieces;
	BitBoard pinned = 0;
	while (sliders)
	{
		uint32_t slider = popLSB(sliders);
		BitBoard betweenBB = attacks::inBetweenBB(kingIdx, slider);
		BitBoard between = board.getAllPieces() & betweenBB;
		if (between == 0)
		{
			checkers |= (1ull << slider);
			moveMask &= (betweenBB | (1ull << slider));
		}
		else if ((between & (between - 1)) == 0)
		{
			pinned |= between & (0 - between);
		}
	}

	/*printBB(checkBB);
	printBB(moveMask);
	printBB(checkers);
	printBB(pinned);*/
	
	return {checkBB, moveMask, checkers, pinned};
}

bool canTakeEP(BitBoard eastRay, BitBoard westRay, BitBoard kingBB, BitBoard enemyAttackers, BitBoard allPieces)
{
	BitBoard eastBlockers = eastRay & allPieces;
	if (eastBlockers == 0)
		return true;

	BitBoard westBlockers = westRay & allPieces;
	if (westBlockers == 0)
		return true;

	BitBoard closestEastBlocker = eastBlockers & (0 - eastBlockers);
	BitBoard other;
	if (closestEastBlocker & kingBB)
	{
		other = enemyAttackers;
	}
	else if (closestEastBlocker & enemyAttackers)
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

template<MoveGenType type, Color color>
Move* genMoves(const Board& board, Move* moves, const CheckInfo& checkInfo);

template<MoveGenType type>
Move* genMoves(const Board& board, Move* moves, const CheckInfo& checkInfo)
{
	if (board.currPlayer() == Color::WHITE)
	{
		return genMoves<type, Color::WHITE>(board, moves, checkInfo);
	}
	else
	{
		return genMoves<type, Color::BLACK>(board, moves, checkInfo);
	}
}

template Move* genMoves<MoveGenType::LEGAL>(const Board& board, Move* moves, const CheckInfo& checkInfo);
template Move* genMoves<MoveGenType::CAPTURES>(const Board& board, Move* moves, const CheckInfo& checkInfo);

template<MoveGenType type, Color color>
void genKingMoves(const Board& board, Move*& moves, BitBoard checkBB);

template<MoveGenType type, Color color>
void genQueenMoves(const Board& board, Move*& moves, BitBoard pinned, BitBoard moveMask);

template<MoveGenType type, Color color>
void genRookMoves(const Board& board, Move*& moves, BitBoard pinned, BitBoard moveMask);

template<MoveGenType type, Color color>
void genBishopMoves(const Board& board, Move*& moves, BitBoard pinned, BitBoard moveMask);

template<MoveGenType type, Color color>
void genKnightMoves(const Board& board, Move*& moves, BitBoard pinned, BitBoard moveMask);

template<MoveGenType type, Color color>
void genPawnMoves(const Board& board, Move*& moves, BitBoard pinned, BitBoard moveMask);



template<MoveGenType type, Color color>
Move* genMoves(const Board& board, Move* moves, const CheckInfo& checkInfo)
{
	if ((checkInfo.checkers & (checkInfo.checkers - 1)) == 0)
	{
		genPawnMoves<type, color>(board, moves, checkInfo.pinned, checkInfo.moveMask);
		genKnightMoves<type, color>(board, moves, checkInfo.pinned, checkInfo.moveMask);
		genBishopMoves<type, color>(board, moves, checkInfo.pinned, checkInfo.moveMask);
		genRookMoves<type, color>(board, moves, checkInfo.pinned, checkInfo.moveMask);
		genQueenMoves<type, color>(board, moves, checkInfo.pinned, checkInfo.moveMask);
	}
	genKingMoves<type, color>(board, moves, checkInfo.checkBB);
	return moves;
}

template<MoveGenType type, Color color>
void genKingMoves(const Board& board, Move*& moves, BitBoard checkBB)
{
	BitBoard usBB = board.getColor(color);
	BitBoard oppBB = board.getColor(flip(color));
	BitBoard kingBB = board.getPieces(PieceType::KING);
	kingBB &= usBB;
	uint32_t kingIdx = getLSB(kingBB);

	BitBoard kingAttacks = attacks::getKingAttacks(kingIdx);
	kingAttacks &= ~usBB;
	kingAttacks &= ~checkBB;
	if constexpr (type == MoveGenType::CAPTURES)
		kingAttacks &= oppBB;
	while (kingAttacks)
	{
		uint32_t dst = popLSB(kingAttacks);
		*moves++ = Move(kingIdx, dst, MoveType::NONE);
	}

	BitBoard occupied = board.getAllPieces();

	uint32_t kscBit = 1 << (2 * static_cast<int>(color));
	uint32_t qscBit = 2 << (2 * static_cast<int>(color));
	if constexpr (type == MoveGenType::LEGAL)
	{
		if (!(attacks::kscCheckSquares<color>() & checkBB) && !(attacks::kscBlockSquares<color>() & occupied) && (board.castlingRights() & kscBit))
		{
			*moves++ = Move(kingIdx, kingIdx + 2, MoveType::CASTLE);
		}
		
		if (!(attacks::qscCheckSquares<color>() & checkBB) && !(attacks::qscBlockSquares<color>() & occupied) && (board.castlingRights() & qscBit))
		{
			*moves++ = Move(kingIdx, kingIdx - 2, MoveType::CASTLE);
		}
	}
}

template<MoveGenType type, Color color>
void genQueenMoves(const Board& board, Move*& moves, BitBoard pinned, BitBoard moveMask)
{
	BitBoard usBB = board.getColor(color);
	BitBoard oppBB = board.getColor(flip(color));
	BitBoard queenBB = board.getPieces(PieceType::QUEEN);
	BitBoard allPieces = board.getAllPieces();
	queenBB &= usBB;
	
	uint32_t kingIdx = getLSB(board.getPieces(PieceType::KING) & usBB);
	
	while (queenBB)
	{
		uint32_t queenIdx = popLSB(queenBB);
		BitBoard queenAttacks = attacks::getQueenAttacks(queenIdx, allPieces);
		queenAttacks &= ~usBB;
		queenAttacks &= moveMask;
		if (pinned & (1ull << queenIdx))
		{
			queenAttacks &= attacks::alignedBB(kingIdx, queenIdx);
		}
		if constexpr (type == MoveGenType::CAPTURES)
			queenAttacks &= oppBB;
		while (queenAttacks)
		{
			uint32_t dst = popLSB(queenAttacks);
			*moves++ = Move(queenIdx, dst, MoveType::NONE);
		}
	}
}

template<MoveGenType type, Color color>
void genRookMoves(const Board& board, Move*& moves, BitBoard pinned, BitBoard moveMask)
{
	BitBoard usBB = board.getColor(color);
	BitBoard oppBB = board.getColor(flip(color));
	BitBoard rookBB = board.getPieces(PieceType::ROOK);
	BitBoard allPieces = board.getAllPieces();
	rookBB &= usBB;
	
	uint32_t kingIdx = getLSB(board.getPieces(PieceType::KING) & usBB);
	
	while (rookBB)
	{
		uint32_t rookIdx = popLSB(rookBB);
		BitBoard rookAttacks = attacks::getRookAttacks(rookIdx, allPieces);
		rookAttacks &= ~usBB;
		rookAttacks &= moveMask;
		if (pinned & (1ull << rookIdx))
		{
			rookAttacks &= attacks::alignedBB(kingIdx, rookIdx);
		}
		if constexpr (type == MoveGenType::CAPTURES)
			rookAttacks &= oppBB;
		while (rookAttacks)
		{
			uint32_t dst = popLSB(rookAttacks);
			*moves++ = Move(rookIdx, dst, MoveType::NONE);
		}
	}
}

template<MoveGenType type, Color color>
void genBishopMoves(const Board& board, Move*& moves, BitBoard pinned, BitBoard moveMask)
{
	BitBoard usBB = board.getColor(color);
	BitBoard oppBB = board.getColor(color);
	BitBoard bishopBB = board.getPieces(PieceType::BISHOP);
	BitBoard allPieces = board.getAllPieces();
	bishopBB &= usBB;
	
	uint32_t kingIdx = getLSB(board.getPieces(PieceType::KING) & usBB);
	
	while (bishopBB)
	{
		uint32_t bishopIdx = popLSB(bishopBB);
		BitBoard bishopAttacks = attacks::getBishopAttacks(bishopIdx, allPieces);
		bishopAttacks &= ~usBB;
		bishopAttacks &= moveMask;
		if (pinned & (1ull << bishopIdx))
		{
			bishopAttacks &= attacks::alignedBB(kingIdx, bishopIdx);
		}
		if constexpr (type == MoveGenType::CAPTURES)
			bishopAttacks &= oppBB;
		while (bishopAttacks)
		{
			uint32_t dst = popLSB(bishopAttacks);
			*moves++ = Move(bishopIdx, dst, MoveType::NONE);
		}
	}
}

template<MoveGenType type, Color color>
void genKnightMoves(const Board& board, Move*& moves, BitBoard pinned, BitBoard moveMask)
{
	BitBoard usBB = board.getColor(color);
	BitBoard oppBB = board.getColor(flip(color));
	BitBoard knightBB = board.getPieces(PieceType::KNIGHT);
	knightBB &= usBB;
	knightBB &= ~pinned;
	
	while (knightBB)
	{
		uint32_t knightIdx = popLSB(knightBB);
		BitBoard knightAttacks = attacks::getKnightAttacks(knightIdx);
		knightAttacks &= moveMask;
		knightAttacks &= ~usBB;
		if constexpr (type == MoveGenType::CAPTURES)
			knightAttacks &= oppBB;
		while (knightAttacks)
		{
			uint32_t dst = popLSB(knightAttacks);
			*moves++ = Move(knightIdx, dst, MoveType::NONE);
		}
	}
}

template<MoveGenType type, Color color>
void genPawnMoves(const Board& board, Move*& moves, BitBoard pinned, BitBoard moveMask)
{
	BitBoard usBB = board.getColor(color);
	BitBoard oppBB = board.getColor(flip(color));
	BitBoard pawns = board.getPieces(PieceType::PAWN) & usBB;
	BitBoard allPieces = board.getAllPieces();
	
	uint32_t kingIdx = getLSB(board.getPieces(PieceType::KING) & usBB);

	BitBoard pinnedPawns = pinned & pawns;
	
	pawns ^= pinnedPawns;
	

	if constexpr (type == MoveGenType::LEGAL)
	{
		BitBoard pawnPushes = attacks::getPawnBBPushes<color>(pawns | (pinnedPawns & (FILE_A << (kingIdx & 7))));
		pawnPushes &= ~allPieces;
		
		BitBoard doublePushes = attacks::getPawnBBPushes<color>(pawnPushes & nthRank<color, 2>());
		doublePushes &= ~allPieces;
		doublePushes &= moveMask;
		
		pawnPushes &= moveMask;
	
		BitBoard promotions = pawnPushes & nthRank<color, 7>();
		
		pawnPushes ^= promotions;
		
		while (pawnPushes)
		{
			uint32_t push = popLSB(pawnPushes);
			*moves++ = Move(push - attacks::pawnPushOffset<color>(), push, MoveType::NONE);
		}
	
		while (doublePushes)
		{
			uint32_t dPush = popLSB(doublePushes);
			*moves++ = Move(dPush - 2 * attacks::pawnPushOffset<color>(), dPush, MoveType::NONE);
		}
	
		while (promotions)
		{
			uint32_t promotion = popLSB(promotions);
			*moves++ = Move(promotion - attacks::pawnPushOffset<color>(), promotion, MoveType::PROMOTION, Promotion::QUEEN);
			*moves++ = Move(promotion - attacks::pawnPushOffset<color>(), promotion, MoveType::PROMOTION, Promotion::ROOK);
			*moves++ = Move(promotion - attacks::pawnPushOffset<color>(), promotion, MoveType::PROMOTION, Promotion::BISHOP);
			*moves++ = Move(promotion - attacks::pawnPushOffset<color>(), promotion, MoveType::PROMOTION, Promotion::KNIGHT);
		}
	}



	BitBoard eastCaptures = attacks::getPawnBBEastAttacks<color>(pawns | (pinnedPawns & attacks::getRay(kingIdx, attacks::pawnEastCaptureDir<color>())));
	if (board.epSquare() != -1)
	{
		if ((eastCaptures & (1ull << board.epSquare())) && ((1ull << (board.epSquare() - attacks::pawnPushOffset<color>())) & moveMask))
		{
			if (canTakeEP(
				attacks::getRay(board.epSquare() - attacks::pawnPushOffset<color>(), Direction::EAST),
				attacks::getRay(board.epSquare() - 1 - attacks::pawnPushOffset<color>(), Direction::WEST),
				1ull << kingIdx,
				(board.getPieces(PieceType::ROOK) & oppBB) | (board.getPieces(PieceType::QUEEN) & oppBB),
				allPieces
			))
			{
				*moves++ = Move(board.epSquare() - attacks::pawnPushOffset<color>() - 1, board.epSquare(), MoveType::ENPASSANT);
			}
		}
	}
	eastCaptures &= moveMask;

	eastCaptures &= oppBB;
	
	BitBoard promotions = eastCaptures & nthRank<color, 7>();
	eastCaptures ^= promotions;

	while (eastCaptures)
	{
		uint32_t capture = popLSB(eastCaptures);
		*moves++ = Move(capture - attacks::pawnPushOffset<color>() - 1, capture, MoveType::NONE);
	}

	while (promotions)
	{
		uint32_t promotion = popLSB(promotions);
		*moves++ = Move(promotion - attacks::pawnPushOffset<color>() - 1, promotion, MoveType::PROMOTION, Promotion::QUEEN);
		*moves++ = Move(promotion - attacks::pawnPushOffset<color>() - 1, promotion, MoveType::PROMOTION, Promotion::ROOK);
		*moves++ = Move(promotion - attacks::pawnPushOffset<color>() - 1, promotion, MoveType::PROMOTION, Promotion::BISHOP);
		*moves++ = Move(promotion - attacks::pawnPushOffset<color>() - 1, promotion, MoveType::PROMOTION, Promotion::KNIGHT);
	}
	

	BitBoard westCaptures = attacks::getPawnBBWestAttacks<color>(pawns | (pinnedPawns & attacks::getRay(kingIdx, attacks::pawnWestCaptureDir<color>())));
	if (board.epSquare() != -1)
	{
		if ((westCaptures & (1ull << board.epSquare())) && ((1ull << (board.epSquare() - attacks::pawnPushOffset<color>())) & moveMask))
		{
			if (canTakeEP(
				attacks::getRay(board.epSquare() + 1 - attacks::pawnPushOffset<color>(), Direction::EAST),
				attacks::getRay(board.epSquare() - attacks::pawnPushOffset<color>(), Direction::WEST),
				1ull << kingIdx,
				(board.getPieces(PieceType::ROOK) & oppBB) | (board.getPieces(PieceType::QUEEN) & oppBB),
				allPieces
			))
			{
				*moves++ = Move(board.epSquare() - attacks::pawnPushOffset<color>() + 1, board.epSquare(), MoveType::ENPASSANT);
			}
		}
	}
	westCaptures &= moveMask;

	westCaptures &= oppBB;
	
	promotions = westCaptures & nthRank<color, 7>();
	westCaptures ^= promotions;

	while (westCaptures)
	{
		uint32_t capture = popLSB(westCaptures);
		*moves++ = Move(capture - attacks::pawnPushOffset<color>() + 1, capture, MoveType::NONE);
	}

	while (promotions)
	{
		uint32_t promotion = popLSB(promotions);
		*moves++ = Move(promotion - attacks::pawnPushOffset<color>() + 1, promotion, MoveType::PROMOTION, Promotion::QUEEN);
		*moves++ = Move(promotion - attacks::pawnPushOffset<color>() + 1, promotion, MoveType::PROMOTION, Promotion::ROOK);
		*moves++ = Move(promotion - attacks::pawnPushOffset<color>() + 1, promotion, MoveType::PROMOTION, Promotion::BISHOP);
		*moves++ = Move(promotion - attacks::pawnPushOffset<color>() + 1, promotion, MoveType::PROMOTION, Promotion::KNIGHT);
	}
}

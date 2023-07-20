#include "move_ordering.h"

#include <climits>

static const int MVV_LVA[7][7] = {
	// none
	{0, 0, 0, 0, 0, 0, 0}, // none x none, none x K, none x Q, none x R, none x B, none x N, none x P
	// king
	{0, 0, 50, 40, 30, 20, 10}, // K x none, K x K, K x Q, K x R, K x B, K x N, K x P
	// queen
	{0, 0, 51, 41, 31, 21, 11}, // Q x none, Q x K, Q x Q, Q x R, Q x B, Q x N, Q x P
	// rook
	{0, 0, 52, 42, 32, 22, 12}, // R x none, R x K, R x Q, R x R, R x B, R x N, R x P
	// bishop
	{0, 0, 53, 43, 33, 23, 13}, // B x none, B x K, B x Q, B x R, B x B, B x N, B x P
	// knight
	{0, 0, 54, 44, 34, 24, 14}, // N x none, N x K, N x Q, N x R, N x B, N x N, N x P
	// pawn
	{0, 0, 55, 45, 35, 25, 15}  // P x none, P x K, P x Q, P x R, P x B, P x N, P x P
};

static const int PROMO_BONUS[4] = {
	// queen
	400,
	// rook
	300,
	// bishop
	200,
	// knight
	100
};

MoveOrdering::MoveOrdering(const Board& board, Move* begin, Move* end)
	: m_Moves(begin), m_Size(static_cast<uint32_t>(end - begin))
{
	for (uint32_t i = 0; i < m_Size; i++)
	{
		int score = 0;
		Move move = begin[i];

		bool isCapture = static_cast<bool>(board.getPieceAt(move.dstPos()));
		bool isPromotion = move.type() == MoveType::PROMOTION;

		if (isCapture)
		{
			int srcPiece = board.getPieceAt(move.srcPos()) & PIECE_TYPE_MASK;
			int dstPiece = board.getPieceAt(move.dstPos()) & PIECE_TYPE_MASK;
			score = CAPTURE_BONUS + MVV_LVA[srcPiece][dstPiece];
		}

		if (isPromotion)
		{
			score += PROMO_BONUS[static_cast<int>(move.promotion()) >> 14];
		}

		m_MoveScores[i] = score;
	}
}

MoveOrdering::MoveOrdering(const Board& board, Move* begin, Move* end, Move (&killers)[2], int (&history)[4096])
	: m_Moves(begin), m_Size(static_cast<uint32_t>(end - begin))
{
	for (uint32_t i = 0; i < m_Size; i++)
	{
		int score = 0;
		Move move = begin[i];

		bool isCapture = static_cast<bool>(board.getPieceAt(move.dstPos()));
		bool isPromotion = move.type() == MoveType::PROMOTION;

		if (isCapture)
		{
			int srcPiece = board.getPieceAt(move.srcPos()) & PIECE_TYPE_MASK;
			int dstPiece = board.getPieceAt(move.dstPos()) & PIECE_TYPE_MASK;
			score = CAPTURE_BONUS + MVV_LVA[srcPiece][dstPiece];
		}

		if (isPromotion)
		{
			score += PROMO_BONUS[static_cast<int>(move.promotion()) >> 14];
		}

		if (!isCapture && !isPromotion)
		{
			if (move == killers[0] || move == killers[1])
				score = KILLER_BONUS;
			else
				score = HISTORY_BONUS + history[move.fromTo()];
		}

		m_MoveScores[i] = score;
	}
}

Move MoveOrdering::selectMove(uint32_t index)
{
	int bestScore = INT_MIN;
	uint32_t bestIndex = index;
	for (uint32_t i = index; i < m_Size; i++)
	{
		if (m_MoveScores[i] > bestScore)
		{
			bestScore = m_MoveScores[i];
			bestIndex = i;
		}
	}

	std::swap(m_Moves[bestIndex], m_Moves[index]);
	std::swap(m_MoveScores[bestIndex], m_MoveScores[index]);

	return m_Moves[index];
}
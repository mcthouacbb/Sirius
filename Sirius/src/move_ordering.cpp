#include "move_ordering.h"

#include <climits>

namespace
{

int mvvLva(const Board& board, Move move)
{
    int srcPiece = static_cast<int>(getPieceType(board.pieceAt(move.fromSq())));
    int dstPiece = static_cast<int>(move.type() == MoveType::ENPASSANT ?
        PieceType::PAWN :
        getPieceType(board.pieceAt(move.toSq()))
    );
    return 10 * dstPiece - srcPiece + 15;
}

int promotionBonus(Move move)
{
    return 1 + (static_cast<int>(move.promotion()) >> 14);
}

}

bool moveIsQuiet(const Board& board, Move move)
{
	return move.type() != MoveType::PROMOTION &&
		move.type() != MoveType::ENPASSANT &&
		board.pieceAt(move.toSq()) == Piece::NONE;
}

bool moveIsCapture(const Board& board, Move move)
{
    return move.type() == MoveType::ENPASSANT ||
        board.pieceAt(move.toSq()) != Piece::NONE;
}

MoveOrdering::MoveOrdering(const Board& board, MoveList& moves, Move hashMove, const History& history)
    : m_Moves(moves)
{
    for (uint32_t i = 0; i < m_Moves.size(); i++)
    {
        int score = 0;
        Move move = m_Moves[i];

        if (move == hashMove)
        {
            m_MoveScores[i] = 10000000;
            continue;
        }

        bool isCapture = moveIsCapture(board, move);
        bool isPromotion = move.type() == MoveType::PROMOTION;
        score = history.getNoisyStats(ExtMove::from(board, move));
        if (isCapture)
            score += mvvLva(board, move);
        if (isPromotion)
            score += 100 * promotionBonus(move);

        m_MoveScores[i] = score;
    }
}

MoveOrdering::MoveOrdering(const Board& board, MoveList& moves, Move hashMove, const std::array<Move, 2>& killers, std::span<const CHEntry* const> contHistEntries, const History& history, int badNoisyMargin)
    : m_Moves(moves)
{
    for (uint32_t i = 0; i < m_Moves.size(); i++)
    {
        int score = 0;
        Move move = m_Moves[i];

        if (move == hashMove)
        {
            m_MoveScores[i] = 1000000;
            continue;
        }

        bool isCapture = moveIsCapture(board, move);
        bool isPromotion = move.type() == MoveType::PROMOTION;

        if (isCapture)
        {
            score = history.getNoisyStats(ExtMove::from(board, move)) + CAPTURE_SCORE * board.see(move, badNoisyMargin) + mvvLva(board, move);
        }
        else if (isPromotion)
        {
            score = history.getNoisyStats(ExtMove::from(board, move)) + PROMOTION_SCORE + promotionBonus(move);
        }
        else if (move == killers[0] || move == killers[1])
            score = KILLER_SCORE + (move == killers[0]);
        else
            score = history.getQuietStats(board.threats(), ExtMove::from(board, move), contHistEntries);
        m_MoveScores[i] = score;
    }
}

ScoredMove MoveOrdering::selectMove(uint32_t index)
{
    int bestScore = INT_MIN;
    uint32_t bestIndex = index;
    for (uint32_t i = index; i < m_Moves.size(); i++)
    {
        if (m_MoveScores[i] > bestScore)
        {
            bestScore = m_MoveScores[i];
            bestIndex = i;
        }
    }

    std::swap(m_Moves[bestIndex], m_Moves[index]);
    std::swap(m_MoveScores[bestIndex], m_MoveScores[index]);

    return {m_Moves[index], m_MoveScores[index]};
}

#include "move_ordering.h"

#include <climits>

namespace
{

int mvvLva(const Board& board, Move move)
{
    int srcPiece = static_cast<int>(getPieceType(board.getPieceAt(move.srcPos())));
    int dstPiece = static_cast<int>(getPieceType(board.getPieceAt(move.dstPos())));
    return 10 * dstPiece - srcPiece + 6;
}

int promotionBonus(Move move)
{
    return 1 + (static_cast<int>(move.promotion()) >> 14);
}

}

MoveOrdering::MoveOrdering(const Board& board, Move* begin, Move* end, Move hashMove)
    : m_Moves(begin), m_Size(static_cast<uint32_t>(end - begin))
{
    for (uint32_t i = 0; i < m_Size; i++)
    {
        int score = 0;
        Move move = begin[i];

        if (move == hashMove)
        {
            m_MoveScores[i] = 10000000;
            continue;
        }

        bool isCapture = static_cast<bool>(board.getPieceAt(move.dstPos()));
        bool isPromotion = move.type() == MoveType::PROMOTION;

        if (isCapture)
            score += mvvLva(board, move);
        if (isPromotion)
            score += 100 * promotionBonus(move);

        m_MoveScores[i] = score;
    }
}

MoveOrdering::MoveOrdering(const Board& board, Move* begin, Move* end, Move hashMove, std::array<Move, 2>& killers, std::array<int, 4096>& history)
    : m_Moves(begin), m_Size(static_cast<uint32_t>(end - begin))
{
    for (uint32_t i = 0; i < m_Size; i++)
    {
        int score = 0;
        Move move = begin[i];

        if (move == hashMove)
        {
            m_MoveScores[i] = 1000000;
            continue;
        }

        bool isCapture = static_cast<bool>(board.getPieceAt(move.dstPos()));
        bool isPromotion = move.type() == MoveType::PROMOTION;

        if (isCapture)
        {
            score = CAPTURE_SCORE * board.see_margin(move, 0) + mvvLva(board, move);
        }
        else if (isPromotion)
        {
            score = PROMOTION_SCORE + promotionBonus(move);
        }
        else if (move == killers[0] || move == killers[1])
            score = KILLER_SCORE;
        else
            score = history[move.fromTo()];
        m_MoveScores[i] = score;
    }
}

ExtMove MoveOrdering::selectMove(uint32_t index)
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

    return {m_Moves[index], m_MoveScores[index]};
}

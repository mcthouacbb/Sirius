#include "move_ordering.h"
#include "search.h"

#include <climits>

namespace
{

i32 mvvLva(const Board& board, Move move)
{
    PieceType dstPiece = move.type() == MoveType::ENPASSANT
        ? PieceType::PAWN
        : getPieceType(board.pieceAt(move.toSq()));
    PieceType srcPiece = getPieceType(board.pieceAt(move.fromSq()));
    return static_cast<i32>(dstPiece) * 8 - static_cast<i32>(srcPiece);
}

}

bool moveIsQuiet(const Board& board, Move move)
{
    return move.type() == MoveType::CASTLE
        || (move.type() != MoveType::PROMOTION && move.type() != MoveType::ENPASSANT
            && board.pieceAt(move.toSq()) == Piece::NONE);
}

bool moveIsCapture(const Board& board, Move move)
{
    return move.type() != MoveType::CASTLE
        && (move.type() == MoveType::ENPASSANT || board.pieceAt(move.toSq()) != Piece::NONE);
}

i32 MoveOrdering::scoreMove(Move move) const
{
    if (moveIsCapture(m_Board, move))
    {
        return mvvLva(m_Board, move);
    }
    else
    {
        return -1000;
    }
}

MoveOrdering::MoveOrdering(const Board& board)
    : m_Board(board), m_Curr(0)
{
    genMoves<MoveGenType::NOISY_QUIET>(board, m_Moves);
    for (i32 i = 0; i < m_Moves.size(); i++)
    {
        m_MoveScores[i] = scoreMove(m_Moves[i]);
    }
}

ScoredMove MoveOrdering::selectMove()
{
    if (m_Curr >= m_Moves.size())
        return {Move(), NO_MOVE};
    return selectHighest();
}

ScoredMove MoveOrdering::selectHighest()
{
    i32 bestScore = INT32_MIN;
    u32 bestIndex = m_Curr;
    for (u32 i = m_Curr; i < m_Moves.size(); i++)
    {
        if (m_MoveScores[i] > bestScore)
        {
            bestScore = m_MoveScores[i];
            bestIndex = i;
        }
    }

    std::swap(m_Moves[bestIndex], m_Moves[m_Curr]);
    std::swap(m_MoveScores[bestIndex], m_MoveScores[m_Curr]);

    return {m_Moves[m_Curr], m_MoveScores[m_Curr++]};
}

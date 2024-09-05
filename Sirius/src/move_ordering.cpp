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

int MoveOrdering::scoreMove(Move move) const
{
    bool isCapture = moveIsCapture(m_Board, move);
    bool isPromotion = move.type() == MoveType::PROMOTION;

    if (isCapture)
    {
        int hist = m_History.getNoisyStats(ExtMove::from(m_Board, move));
        return hist + CAPTURE_SCORE * m_Board.see(move, -hist / 32) + mvvLva(m_Board, move);
    }
    else if (isPromotion)
    {
        return m_History.getNoisyStats(ExtMove::from(m_Board, move)) + PROMOTION_SCORE + promotionBonus(move);
    }
    else if (move == m_Killers[0] || move == m_Killers[1])
        return KILLER_SCORE + (move == m_Killers[0]);
    else
        return m_History.getQuietStats(m_Board.threats(), ExtMove::from(m_Board, move), m_ContHistEntries);
}

int MoveOrdering::scoreMoveQSearch(Move move) const
{
    bool isCapture = moveIsCapture(m_Board, move);
    bool isPromotion = move.type() == MoveType::PROMOTION;
    int score = m_History.getNoisyStats(ExtMove::from(m_Board, move));
    if (isCapture)
        score += mvvLva(m_Board, move);
    if (isPromotion)
        score += 100 * promotionBonus(move);

    return score;
}

MoveOrdering::MoveOrdering(const Board& board, MoveList& moves, Move ttMove, const History& history)
    : m_Board(board), m_Moves(moves), m_TTMove(ttMove), m_History(history), m_Curr(0), m_Stage(MovePickStage::QS_TT_MOVE)
{
}

MoveOrdering::MoveOrdering(const Board& board, MoveList& moves, Move ttMove, const std::array<Move, 2>& killers, std::span<const CHEntry* const> contHistEntries, const History& history)
    : m_Board(board), m_Moves(moves), m_TTMove(ttMove),
    m_History(history), m_ContHistEntries(contHistEntries), m_Killers(killers),
    m_Curr(0), m_Stage(MovePickStage::TT_MOVE)
{
}

ScoredMove MoveOrdering::selectMove()
{
    using enum MovePickStage;
    switch (m_Stage)
    {
        case TT_MOVE:
            ++m_Stage;
            if (m_TTMove != Move() && m_Board.isPseudoLegal(m_TTMove))
                return ScoredMove(m_TTMove, 10000000);

            // fallthrough
        case GEN_NOISY_QUIETS:
            ++m_Stage;
            for (uint32_t i = 0; i < m_Moves.size(); i++)
                m_MoveScores[i] = scoreMove(m_Moves[i]);

            // fallthrough
        case NOISY_QUIETS:
            while (m_Curr < m_Moves.size())
            {
                ScoredMove scoredMove = selectHighest();
                if (scoredMove.move != m_TTMove)
                    return scoredMove;
            }
            return {Move(), NO_MOVE};


        case QS_TT_MOVE:
            ++m_Stage;
            if (m_TTMove != Move() && m_Board.isPseudoLegal(m_TTMove) && !moveIsQuiet(m_Board, m_TTMove))
                return ScoredMove(m_TTMove, 10000000);

            // fallthrough
        case QS_GEN_NOISIES:
            ++m_Stage;
            for (uint32_t i = 0; i < m_Moves.size(); i++)
                m_MoveScores[i] = scoreMoveQSearch(m_Moves[i]);

            // fallthrough
        case QS_NOISIES:
            while (m_Curr < m_Moves.size())
            {
                ScoredMove scoredMove = selectHighest();
                if (scoredMove.move != m_TTMove)
                    return scoredMove;
            }
            return {Move(), NO_MOVE};
    }
    if (m_Curr >= m_Moves.size())
        return {Move(), NO_MOVE};
    return selectHighest();
}

ScoredMove MoveOrdering::selectHighest()
{
    int bestScore = INT_MIN;
    uint32_t bestIndex = m_Curr;
    for (uint32_t i = m_Curr; i < m_Moves.size(); i++)
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

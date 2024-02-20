#include "eval.h"

namespace eval
{

int evaluate(const Board& board)
{
    if (!eval::canForceMate(board))
        return SCORE_DRAW;
    return rawEval(board);
}

int rawEval(const Board& board)
{
    Color color = board.sideToMove();
    PackedScore eval = board.evalState().materialPsqt;
    if (board.getPieces(Color::WHITE, PieceType::BISHOP).popcount() >= 2)
        eval += BISHOP_PAIR;
    if (board.getPieces(Color::BLACK, PieceType::BISHOP).popcount() >= 2)
        eval -= BISHOP_PAIR;
    return (color == Color::WHITE ? 1 : -1) * eval::getFullEval(eval.mg(), eval.eg(), board.evalState().phase);
}


}

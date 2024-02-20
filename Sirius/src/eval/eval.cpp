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

    std::array<Bitboard, 2> pawns = {
        board.getPieces(Color::WHITE, PieceType::PAWN),
        board.getPieces(Color::BLACK, PieceType::PAWN)
    };

    Bitboard rooks = board.getPieces(Color::WHITE, PieceType::ROOK);
    while (rooks)
    {
        uint32_t sq = rooks.poplsb();
        if ((pawns[0] & (FILE_A << (sq % 8))).empty())
        {
            if ((pawns[1] & (FILE_A << (sq % 8))).empty())
                eval += OPEN_ROOK[0];
            else
                eval += OPEN_ROOK[1];
        }
    }

    rooks = board.getPieces(Color::BLACK, PieceType::ROOK);
    while (rooks)
    {
        uint32_t sq = rooks.poplsb();
        if ((pawns[1] & (FILE_A << (sq % 8))).empty())
        {
            if ((pawns[0] & (FILE_A << (sq % 8))).empty())
                eval -= OPEN_ROOK[0];
            else
                eval -= OPEN_ROOK[1];
        }
    }

    if (board.getPieces(Color::WHITE, PieceType::BISHOP).popcount() >= 2)
        eval += BISHOP_PAIR;
    if (board.getPieces(Color::BLACK, PieceType::BISHOP).popcount() >= 2)
        eval -= BISHOP_PAIR;
    return (color == Color::WHITE ? 1 : -1) * eval::getFullEval(eval.mg(), eval.eg(), board.evalState().phase);
}


}

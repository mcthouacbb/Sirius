#include "draw.h"

namespace eval
{

bool isImmediateDraw(const Board& board)
{
    Bitboard nonMinorPcs =
        board.getPieces(PieceType::QUEEN) |
        board.getPieces(PieceType::ROOK) |
        board.getPieces(PieceType::PAWN);

    if (nonMinorPcs != 0)
        return false;

    switch (board.getAllPieces().popcount())
    {
        case 2:
            return true;
        case 3:
            return true;
        case 4:
        {
            Bitboard bishops = board.getPieces(PieceType::BISHOP);
            if (bishops.popcount() == 2 && ((bishops & LIGHT_SQUARES).popcount() == 2 || (bishops & LIGHT_SQUARES).empty()))
                return true;
            return false;
        }
        default:
            return false;
    }
}

bool canForceMate(const Board& board)
{
    Bitboard nonMinorPcs =
        board.getPieces(PieceType::QUEEN) |
        board.getPieces(PieceType::ROOK) |
        board.getPieces(PieceType::PAWN);

    if (nonMinorPcs != 0)
        return true;

    Bitboard bishops = board.getPieces(PieceType::BISHOP);
    Bitboard knights = board.getPieces(PieceType::KNIGHT);

    Bitboard white = board.getColor(Color::WHITE);
    Bitboard black = board.getColor(Color::BLACK);

    Bitboard blackBishops = black & bishops;
    Bitboard blackKnights = black & knights;
    Bitboard whiteBishops = white & bishops;
    Bitboard whiteKnights = white & knights;

    if ((whiteBishops & LIGHT_SQUARES) && (whiteBishops & DARK_SQUARES))
        return true;

    if ((blackBishops & LIGHT_SQUARES) && (whiteBishops & DARK_SQUARES))
        return true;

    if (whiteBishops && whiteKnights)
        return true;

    if (blackBishops && blackKnights)
        return true;

    if ((whiteKnights).popcount() > 2 || (blackKnights).popcount() > 2)
        return true;

    return false;
}

}

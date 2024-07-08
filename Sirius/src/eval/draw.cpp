#include "draw.h"

namespace eval
{

bool isImmediateDraw(const Board& board)
{
    Bitboard nonMinorPcs =
        board.pieces(PieceType::QUEEN) |
        board.pieces(PieceType::ROOK) |
        board.pieces(PieceType::PAWN);

    if (nonMinorPcs.any())
        return false;

    switch (board.allPieces().popcount())
    {
        case 2:
            return true;
        case 3:
            return true;
        case 4:
        {
            Bitboard bishops = board.pieces(PieceType::BISHOP);
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
        board.pieces(PieceType::QUEEN) |
        board.pieces(PieceType::ROOK) |
        board.pieces(PieceType::PAWN);

    if (nonMinorPcs.any())
        return true;

    Bitboard bishops = board.pieces(PieceType::BISHOP);
    Bitboard knights = board.pieces(PieceType::KNIGHT);

    Bitboard white = board.pieces(Color::WHITE);
    Bitboard black = board.pieces(Color::BLACK);

    Bitboard blackBishops = black & bishops;
    Bitboard blackKnights = black & knights;
    Bitboard whiteBishops = white & bishops;
    Bitboard whiteKnights = white & knights;

    if ((whiteBishops & LIGHT_SQUARES).any() && (whiteBishops & DARK_SQUARES).any())
        return true;

    if ((blackBishops & LIGHT_SQUARES).any() && (whiteBishops & DARK_SQUARES).any())
        return true;

    if (whiteBishops.any() && whiteKnights.any())
        return true;

    if (blackBishops.any() && blackKnights.any())
        return true;

    if ((whiteKnights).popcount() > 2 || (blackKnights).popcount() > 2)
        return true;

    return false;
}

}

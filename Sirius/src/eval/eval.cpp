#include "eval.h"
#include "../attacks.h"

namespace eval
{

template<Color color, PieceType piece>
PackedScore evaluatePieces(const Board& board)
{
    Bitboard ourPawns = board.getPieces(color, PieceType::PAWN);
    Bitboard theirPawns = board.getPieces(~color, PieceType::PAWN);

    Bitboard mobilityArea = ~attacks::pawnAttacks<~color>(theirPawns);

    PackedScore eval{0, 0};
    Bitboard pieces = board.getPieces(color, piece);
    if constexpr (piece == PieceType::BISHOP)
        if (pieces.multiple())
            eval += BISHOP_PAIR;

    while (pieces)
    {
        uint32_t sq = pieces.poplsb();
        Bitboard attacksBB = attacks::pieceAttacks<piece>(sq, board.getAllPieces());
        eval += MOBILITY[static_cast<int>(piece) - static_cast<int>(PieceType::KNIGHT)][(attacksBB & mobilityArea).popcount()];

        Bitboard fileBB = Bitboard::fileBB(fileOf(sq));

        if constexpr (piece == PieceType::ROOK)
        {
            if ((ourPawns & fileBB).empty())
                eval += (theirPawns & fileBB).any() ? ROOK_SEMI_OPEN : ROOK_OPEN;
        }
    }

    return eval;
}

int evaluate(const Board& board)
{
    if (!eval::canForceMate(board))
        return SCORE_DRAW;

    Color color = board.sideToMove();
    PackedScore eval = board.evalState().materialPsqt;

    eval += evaluatePieces<Color::WHITE, PieceType::KNIGHT>(board) - evaluatePieces<Color::BLACK, PieceType::KNIGHT>(board);
    eval += evaluatePieces<Color::WHITE, PieceType::BISHOP>(board) - evaluatePieces<Color::BLACK, PieceType::BISHOP>(board);
    eval += evaluatePieces<Color::WHITE, PieceType::ROOK>(board) - evaluatePieces<Color::BLACK, PieceType::ROOK>(board);
    eval += evaluatePieces<Color::WHITE, PieceType::QUEEN>(board) - evaluatePieces<Color::BLACK, PieceType::QUEEN>(board);

    return (color == Color::WHITE ? 1 : -1) * eval::getFullEval(eval.mg(), eval.eg(), board.evalState().phase);
}


}

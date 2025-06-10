#pragma once

#include "../bitboard.h"
#include "../board.h"
#include "../defs.h"
#include "eval_state.h"
#include <bitset>
#include <type_traits>

class Board;
struct PawnStructure;
class PawnTable;

namespace eval
{

namespace eval_terms
{

using enum PieceType;

inline bool pawnStructureChanged(const EvalUpdates& updates)
{
    return updates.changedPieces.has(PieceType::PAWN);
}

inline bool pawnShieldStormChanged(const Board& board, const EvalUpdates& updates)
{
    if (!updates.changedPieces.hasAny(PieceSet(PieceType::PAWN, PieceType::KING)))
        return false;

    // pawn moves not near the king cannot change the pawn shield/storm
    if (updates.type == MoveType::NONE && updates.move->movedPiece == PieceType::PAWN)
    {
        int whiteKingFile = std::clamp(board.kingSq(Color::WHITE).file(), FILE_B, FILE_G);
        int blackKingFile = std::clamp(board.kingSq(Color::BLACK).file(), FILE_B, FILE_G);

        int fromFile = updates.move->from.file();
        int toFile = updates.move->to.file();
        if (std::abs(whiteKingFile - fromFile) > 1 && std::abs(blackKingFile - fromFile) > 1
            && std::abs(whiteKingFile - toFile) > 1 && std::abs(blackKingFile - toFile) > 1)
            return false;
    }

    // the king moving vertically cannot change the pawn shield/storm score
    if (updates.type == MoveType::NONE && updates.move->movedPiece == PieceType::KING
        && updates.captured != PieceType::PAWN
        && (updates.move->from - updates.move->to == 8 || updates.move->from - updates.move->to == -8))
        return false;

    return true;
}

inline bool knightOutpostsChanged(const Board& board, const EvalUpdates& updates)
{
    if (!updates.changedPieces.hasAny(PieceSet(PieceType::PAWN, PieceType::KNIGHT)))
        return false;

    // if there are no knights left on the board
    if (board.pieces(PieceType::KNIGHT).empty() && !updates.changedPieces.has(PieceType::KNIGHT))
        return false;

    return true;
}

inline bool bishopPawnsChanged(const Board& board, const EvalUpdates& updates)
{
    if (!updates.changedPieces.hasAny(PieceSet(PieceType::PAWN, PieceType::BISHOP)))
        return false;

    // if there are no bishops left on the board
    if (board.pieces(PieceType::BISHOP).empty() && !updates.changedPieces.has(PieceType::BISHOP))
        return false;

    // bishop moves don't change color complex
    if (updates.type == MoveType::NONE && updates.move->movedPiece == PieceType::BISHOP
        && updates.captured != PieceType::PAWN && updates.captured != PieceType::BISHOP)
        return false;

    // pawn captures don't change color complex
    if (updates.type == MoveType::NONE && updates.move->movedPiece == PieceType::PAWN
        && updates.captured != PieceType::PAWN && updates.captured != PieceType::BISHOP
        && updates.move->from - updates.move->to != 8 && updates.move->from - updates.move->to != -8)
        return false;

    return true;
}

inline bool rookOpenChanged(const Board& board, const EvalUpdates& updates)
{
    if (!updates.changedPieces.hasAny(PieceSet(PieceType::PAWN, PieceType::ROOK)))
        return false;

    // if there are no rooks left on the board
    if (board.pieces(PieceType::ROOK).empty() && !updates.changedPieces.has(PieceType::ROOK))
        return false;

    // a pawn push cannot change the openness of a file
    if (updates.type == MoveType::NONE && updates.move->movedPiece == PieceType::PAWN
        && (updates.move->from - updates.move->to) % 8 == 0)
        return false;

    // moving a rook along a file cannot change the openness of a file
    if (updates.type == MoveType::NONE && updates.move->movedPiece == PieceType::ROOK
        && updates.captured != PieceType::PAWN && updates.captured != PieceType::ROOK
        && (updates.move->from - updates.move->to) % 8 == 0)
        return false;

    return true;
}

inline bool minorBehindPawnChanged(const Board& board, const EvalUpdates& updates)
{
    if (!updates.changedPieces.hasAny(PieceSet(PieceType::PAWN, PieceType::KNIGHT, PieceType::BISHOP)))
        return false;

    // if there are no knights or bishops left on the board
    if ((board.pieces(PieceType::BISHOP) | board.pieces(PieceType::KNIGHT)).empty()
        && !updates.changedPieces.hasAny(PieceSet(PieceType::KNIGHT, PieceType::BISHOP)))
        return false;

    return true;
}

} // namespace eval_terms

void evaluatePawns(const Board& board, PawnStructure& pawnStructure, PawnTable* pawnTable);

template<Color us>
ScorePair evalKingPawnFile(uint32_t file, Bitboard ourPawns, Bitboard theirPawns);

template<Color us>
ScorePair evaluateStormShield(const Board& board);

template<Color us>
ScorePair evaluateKnightOutposts(const Board& board, const PawnStructure& pawnStructure);

template<Color us>
ScorePair evaluateBishopPawns(const Board& board);

template<Color us>
ScorePair evaluateRookOpen(const Board& board);

template<Color us>
ScorePair evaluateMinorBehindPawn(const Board& board);

}

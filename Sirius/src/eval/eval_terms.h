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

    return true;
}

inline bool knightOutpostsChanged(const EvalUpdates& updates)
{
    return updates.changedPieces.hasAny(PieceSet(PieceType::PAWN, PieceType::KNIGHT));
}

inline bool bishopPawnsChanged(const EvalUpdates& updates)
{
    return updates.changedPieces.hasAny(PieceSet(PieceType::PAWN, PieceType::BISHOP));
}

inline bool rookOpenChanged(const EvalUpdates& updates)
{
    if (!updates.changedPieces.hasAny(PieceSet(PieceType::PAWN, PieceType::ROOK)))
        return false;

    // a pawn push cannot change the openness of a file
    if (updates.type == MoveType::NONE && updates.move->movedPiece == PieceType::PAWN
        && (updates.move->from - updates.move->to == 8 || updates.move->from - updates.move->to == -8))
        return false;

    // moving a rook along a file cannot change the openness of a file
    if (updates.type == MoveType::NONE && updates.move->movedPiece == PieceType::ROOK
        && updates.captured != PieceType::PAWN && updates.captured != PieceType::ROOK
        && (updates.move->from - updates.move->to) % 8 == 0)
        return false;

    return true;
}

inline bool minorBehindPawnChanged(const EvalUpdates& updates)
{
    return updates.changedPieces.hasAny(
        PieceSet(PieceType::PAWN, PieceType::KNIGHT, PieceType::BISHOP));
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

#include "pawn_structure.h"
#include "../attacks.h"
#include "eval_constants.h"

namespace eval
{

PawnStructure::PawnStructure(const Board& board)
{
    Bitboard wpawns = board.pieces(Color::WHITE, PieceType::PAWN);
    Bitboard bpawns = board.pieces(Color::BLACK, PieceType::PAWN);
    pawnAttacks[Color::WHITE] = attacks::pawnAttacks<Color::WHITE>(wpawns);
    pawnAttackSpans[Color::WHITE] = attacks::fillUp<Color::WHITE>(pawnAttacks[Color::WHITE]);
    passedPawns = Bitboard(0);

    pawnAttacks[Color::BLACK] = attacks::pawnAttacks<Color::BLACK>(bpawns);
    pawnAttackSpans[Color::BLACK] = attacks::fillUp<Color::BLACK>(pawnAttacks[Color::BLACK]);
    passedPawns = Bitboard(0);
}

PackedScore PawnStructure::evaluate(const Board& board)
{
    score = evaluate<Color::WHITE>(board) - evaluate<Color::BLACK>(board);
    return score;
}

template<Color us>
PackedScore PawnStructure::evaluate(const Board& board)
{
    Bitboard ourPawns = board.pieces(us, PieceType::PAWN);

    PackedScore eval{0, 0};

    Bitboard pawns = ourPawns;
    while (pawns.any())
    {
        Square sq = pawns.poplsb();
        if (board.isPassedPawn(sq))
            passedPawns |= Bitboard::fromSquare(sq);
        if (board.isIsolatedPawn(sq))
            eval += ISOLATED_PAWN[sq.file()];
    }

    Bitboard phalanx = ourPawns & ourPawns.west();
    while (phalanx.any())
        eval += PAWN_PHALANX[phalanx.poplsb().relativeRank<us>()];

    Bitboard defended = ourPawns & pawnAttacks[us];
    while (defended.any())
        eval += DEFENDED_PAWN[defended.poplsb().relativeRank<us>()];

    return eval;
}

}

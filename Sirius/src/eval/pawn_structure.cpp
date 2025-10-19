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

    pawnAttacks[Color::BLACK] = attacks::pawnAttacks<Color::BLACK>(bpawns);
    pawnAttackSpans[Color::BLACK] = attacks::fillUp<Color::BLACK>(pawnAttacks[Color::BLACK]);

    passedPawns = EMPTY_BB;
}

ScorePair PawnStructure::evaluate(const Board& board)
{
    score = evaluate<Color::WHITE>(board) - evaluate<Color::BLACK>(board);
    return score;
}

template<Color us>
ScorePair PawnStructure::evaluate(const Board& board)
{
    constexpr Color them = ~us;
    Bitboard ourPawns = board.pieces(us, PieceType::PAWN);
    Bitboard theirPawns = board.pieces(them, PieceType::PAWN);

    ScorePair eval = ScorePair(0, 0);

    Bitboard pawns = ourPawns;
    while (pawns.any())
    {
        Square sq = pawns.poplsb();
        Square push = sq + attacks::pawnPushOffset<us>();
        Bitboard attacks = attacks::pawnAttacks(us, sq);
        Bitboard neighbors = attacks::isolatedPawnMask(sq) & ourPawns;
        Bitboard support = attacks::passedPawnMask(them, push) & neighbors;
        Bitboard threats = attacks & theirPawns;
        Bitboard pushThreats = attacks::pawnPushes<us>(attacks) & theirPawns;
        Bitboard defenders = attacks::pawnAttacks(them, sq) & ourPawns;
        Bitboard phalanx = attacks::pawnAttacks(them, push) & ourPawns;
        Bitboard stoppers = attacks::passedPawnMask(us, sq) & theirPawns;

        bool exposed = (stoppers & Bitboard::fileBB(sq.file())).empty();
        bool blocked = theirPawns.has(push);
        bool doubled = ourPawns.has(push);
        bool isolated = neighbors.empty();
        bool backwards = (blocked || pushThreats.any()) && support.empty();

        if (stoppers.empty())
            passedPawns |= Bitboard::fromSquare(sq);
        else if (stoppers == (pushThreats | threats) && phalanx.popcount() >= pushThreats.popcount())
        {
            bool defended = defenders.popcount() >= threats.popcount();
            eval += CANDIDATE_PASSER[defended][sq.relativeRank<us>()];
        }

        if (doubled && threats.empty())
            eval += DOUBLED_PAWN[std::min(sq.file(), sq.file() ^ 7)];

        if (threats.empty() && isolated)
            eval += ISOLATED_PAWN[std::min(sq.file(), sq.file() ^ 7)] + ISOLATED_EXPOSED * exposed;
        else if (backwards)
            eval += BACKWARDS_PAWN[sq.relativeRank<us>()] + BACKWARDS_EXPOSED * exposed;
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

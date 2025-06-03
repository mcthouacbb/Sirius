#include "eval_terms.h"

#include "../attacks.h"
#include "../board.h"

#include "eval_constants.h"
#include "pawn_structure.h"
#include "pawn_table.h"

#include <algorithm>

namespace eval
{

void evaluatePawns(const Board& board, PawnStructure& pawnStructure, PawnTable* pawnTable)
{
    if (pawnTable)
    {
        const auto& entry = pawnTable->probe(board.pawnKey());
        if (entry.pawnKey == board.pawnKey())
        {
            pawnStructure = entry.pawnStructure;
            return;
        }
    }

    pawnStructure = PawnStructure(board);
    pawnStructure.evaluate(board);

    if (pawnTable)
    {
        PawnEntry replace = {board.pawnKey(), pawnStructure};
        pawnTable->store(replace);
    }
}

template<Color us>
ScorePair evalKingPawnFile(uint32_t file, Bitboard ourPawns, Bitboard theirPawns)
{
    constexpr Color them = ~us;

    ScorePair eval = ScorePair(0, 0);
    int edgeDist = std::min(file, 7 - file);
    {
        Bitboard filePawns = ourPawns & Bitboard::fileBB(file);
        int rank = 0;
        bool blocked = false;
        if (filePawns.any())
        {
            Square filePawn = us == Color::WHITE ? filePawns.msb() : filePawns.lsb();
            rank = filePawn.relativeRank<them>();
            blocked = theirPawns.has(filePawn + attacks::pawnPushOffset<us>());
        }
        eval += PAWN_STORM[blocked][edgeDist][rank];
    }
    {
        Bitboard filePawns = theirPawns & Bitboard::fileBB(file);
        int rank = filePawns.any()
            ? (us == Color::WHITE ? filePawns.msb() : filePawns.lsb()).relativeRank<them>()
            : 0;
        eval += PAWN_SHIELD[edgeDist][rank];
    }
    return eval;
}

template<Color us>
ScorePair evaluateStormShield(const Board& board)
{
    constexpr Color them = ~us;
    ScorePair eval = ScorePair(0, 0);
    Bitboard ourPawns = board.pieces(us, PieceType::PAWN);
    Bitboard theirPawns = board.pieces(them, PieceType::PAWN);
    Square theirKing = board.kingSq(them);

    uint32_t middleFile = std::clamp(theirKing.file(), FILE_B, FILE_G);
    for (uint32_t file = middleFile - 1; file <= middleFile + 1; file++)
        eval += evalKingPawnFile<us>(file, ourPawns, theirPawns);

    return eval;
}

template<Color us>
ScorePair evaluateKnightOutposts(const Board& board, const PawnStructure& pawnStructure)
{
    constexpr Color them = ~us;
    Bitboard outpostRanks = RANK_4_BB | RANK_5_BB | (us == Color::WHITE ? RANK_6_BB : RANK_3_BB);
    Bitboard outposts =
        outpostRanks & ~pawnStructure.pawnAttackSpans[them] & pawnStructure.pawnAttacks[us];
    return KNIGHT_OUTPOST * (board.pieces(us, PieceType::KNIGHT) & outposts).popcount();
}

template<Color us>
ScorePair evaluateBishopPawns(const Board& board)
{
    Bitboard bishops = board.pieces(us, PieceType::BISHOP);

    ScorePair eval = ScorePair(0, 0);
    while (bishops.any())
    {
        Square sq = bishops.poplsb();
        bool lightSquare = LIGHT_SQUARES_BB.has(sq);
        Bitboard sameColorPawns =
            board.pieces(us, PieceType::PAWN) & (lightSquare ? LIGHT_SQUARES_BB : DARK_SQUARES_BB);
        eval += BISHOP_PAWNS[std::min(sameColorPawns.popcount(), 6u)];
    }
    return eval;
}

template<Color us>
ScorePair evaluateRookOpen(const Board& board)
{
    constexpr Color them = ~us;
    Bitboard ourPawns = board.pieces(us, PieceType::PAWN);
    Bitboard theirPawns = board.pieces(them, PieceType::PAWN);
    Bitboard rooks = board.pieces(us, PieceType::ROOK);

    ScorePair eval = ScorePair(0, 0);
    while (rooks.any())
    {
        Bitboard fileBB = Bitboard::fileBB(rooks.poplsb().file());
        if ((ourPawns & fileBB).empty())
            eval += (theirPawns & fileBB).any() ? ROOK_OPEN[1] : ROOK_OPEN[0];
    }
    return eval;
}

template<Color us>
ScorePair evaluateMinorBehindPawn(const Board& board)
{
    constexpr Color them = ~us;

    Bitboard pawns = board.pieces(PieceType::PAWN);
    Bitboard minors = board.pieces(us, PieceType::KNIGHT) | board.pieces(us, PieceType::BISHOP);

    Bitboard shielded = minors & attacks::pawnPushes<them>(pawns);
    return MINOR_BEHIND_PAWN * shielded.popcount();
}

template ScorePair evalKingPawnFile<Color::WHITE>(uint32_t file, Bitboard ourPawns, Bitboard theirPawns);
template ScorePair evalKingPawnFile<Color::BLACK>(uint32_t file, Bitboard ourPawns, Bitboard theirPawns);

template ScorePair evaluateStormShield<Color::WHITE>(const Board& board);
template ScorePair evaluateStormShield<Color::BLACK>(const Board& board);

template ScorePair evaluateKnightOutposts<Color::WHITE>(
    const Board& board, const PawnStructure& pawnStructure);
template ScorePair evaluateKnightOutposts<Color::BLACK>(
    const Board& board, const PawnStructure& pawnStructure);

template ScorePair evaluateBishopPawns<Color::WHITE>(const Board& board);
template ScorePair evaluateBishopPawns<Color::BLACK>(const Board& board);

template ScorePair evaluateRookOpen<Color::WHITE>(const Board& board);
template ScorePair evaluateRookOpen<Color::BLACK>(const Board& board);

template ScorePair evaluateMinorBehindPawn<Color::WHITE>(const Board& board);
template ScorePair evaluateMinorBehindPawn<Color::BLACK>(const Board& board);

}

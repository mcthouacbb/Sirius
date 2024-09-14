#include "eval_terms.h"

#include "../board.h"
#include "../attacks.h"

#include "pawn_structure.h"
#include "pawn_table.h"
#include "eval_constants.h"

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
PackedScore evalKingPawnFile(uint32_t file, Bitboard ourPawns, Bitboard theirPawns)
{
    constexpr Color them = ~us;

    PackedScore eval{0, 0};
    int edgeDist = std::min(file, 7 - file);
    {
        Bitboard filePawns = ourPawns & Bitboard::fileBB(file);
        int rank = filePawns.any() ?
            (us == Color::WHITE ? filePawns.msb() : filePawns.lsb()).relativeRank<them>() :
            0;
        bool blocked = (theirPawns & Bitboard::fromSquare(Square(rank + attacks::pawnPushOffset<us>(), file))).any();
        eval += PAWN_STORM[blocked][edgeDist][rank];
    }
    {
        Bitboard filePawns = theirPawns & Bitboard::fileBB(file);
        int rank = filePawns.any() ?
            (us == Color::WHITE ? filePawns.msb() : filePawns.lsb()).relativeRank<them>() :
            0;
        eval += PAWN_SHIELD[edgeDist][rank];
    }
    return eval;
}

template<Color us>
PackedScore evaluateStormShield(const Board& board)
{
    constexpr Color them = ~us;
    PackedScore eval{0, 0};
    Bitboard ourPawns = board.pieces(us, PieceType::PAWN);
    Bitboard theirPawns = board.pieces(them, PieceType::PAWN);
    Square theirKing = board.kingSq(them);

    uint32_t leftFile = std::clamp(theirKing.file() - 1, FILE_A, FILE_F);
    uint32_t rightFile = std::clamp(theirKing.file() + 1, FILE_C, FILE_H);
    for (uint32_t file = leftFile; file <= rightFile; file++)
        eval += evalKingPawnFile<us>(file, ourPawns, theirPawns);

    return eval;
}

template<Color us>
PackedScore evaluateKnightOutposts(const Board& board, const PawnStructure& pawnStructure)
{
    constexpr Color them = ~us;
    Bitboard outpostRanks = RANK_4_BB | RANK_5_BB | (us == Color::WHITE ? RANK_6_BB : RANK_3_BB);
    Bitboard outposts = outpostRanks & ~pawnStructure.pawnAttackSpans[them] & pawnStructure.pawnAttacks[us];
    return KNIGHT_OUTPOST * (board.pieces(us, PieceType::KNIGHT) & outposts).popcount();
}

template<Color us>
PackedScore evaluateBishopPawns(const Board& board)
{
    Bitboard bishops = board.pieces(us, PieceType::BISHOP);

    PackedScore eval{0, 0};
    while (bishops.any())
    {
        Square sq = bishops.poplsb();
        bool lightSquare = (Bitboard::fromSquare(sq) & LIGHT_SQUARES_BB).any();
        Bitboard sameColorPawns = board.pieces(us, PieceType::PAWN) & (lightSquare ? LIGHT_SQUARES_BB : DARK_SQUARES_BB);
        eval += BISHOP_PAWNS[std::min(sameColorPawns.popcount(), 6u)];
    }
    return eval;
}

template<Color us>
PackedScore evaluateRookOpen(const Board& board)
{
    constexpr Color them = ~us;
    Bitboard ourPawns = board.pieces(us, PieceType::PAWN);
    Bitboard theirPawns = board.pieces(them, PieceType::PAWN);
    Bitboard rooks = board.pieces(us, PieceType::ROOK);

    PackedScore eval{0, 0};
    while (rooks.any())
    {
        Bitboard fileBB = Bitboard::fileBB(rooks.poplsb().file());
        if ((ourPawns & fileBB).empty())
            eval += (theirPawns & fileBB).any() ? ROOK_OPEN[1] : ROOK_OPEN[0];
    }
    return eval;
}

template<Color us>
PackedScore evaluateMinorBehindPawn(const Board& board)
{
    Bitboard pawns = board.pieces(PieceType::PAWN);
    Bitboard minors = board.pieces(us, PieceType::KNIGHT) | board.pieces(us, PieceType::BISHOP);

    Bitboard shielded = minors & (us == Color::WHITE ? pawns.south() : pawns.north());
    return MINOR_BEHIND_PAWN * shielded.popcount();
}

template PackedScore evalKingPawnFile<Color::WHITE>(uint32_t file, Bitboard ourPawns, Bitboard theirPawns);
template PackedScore evalKingPawnFile<Color::BLACK>(uint32_t file, Bitboard ourPawns, Bitboard theirPawns);

template PackedScore evaluateStormShield<Color::WHITE>(const Board& board);
template PackedScore evaluateStormShield<Color::BLACK>(const Board& board);

template PackedScore evaluateKnightOutposts<Color::WHITE>(const Board& board, const PawnStructure& pawnStructure);
template PackedScore evaluateKnightOutposts<Color::BLACK>(const Board& board, const PawnStructure& pawnStructure);

template PackedScore evaluateBishopPawns<Color::WHITE>(const Board& board);
template PackedScore evaluateBishopPawns<Color::BLACK>(const Board& board);

template PackedScore evaluateRookOpen<Color::WHITE>(const Board& board);
template PackedScore evaluateRookOpen<Color::BLACK>(const Board& board);

template PackedScore evaluateMinorBehindPawn<Color::WHITE>(const Board& board);
template PackedScore evaluateMinorBehindPawn<Color::BLACK>(const Board& board);

}

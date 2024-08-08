#pragma once

#include <bitset>
#include <type_traits>
#include "../defs.h"
#include "../util/piece_set.h"
// TEMPORARY
#include "../attacks.h"

namespace eval
{

struct EvalTerm
{
    PieceSet deps;
};

namespace eval_terms
{

constexpr EvalTerm pawnStructure = {PieceSet(PieceType::PAWN)};
constexpr EvalTerm passers = {PieceSet(PieceType::PAWN, PieceType::KNIGHT, PieceType::BISHOP, PieceType::ROOK, PieceType::QUEEN, PieceType::KING)};
constexpr EvalTerm pawnShieldStorm = {PieceSet(PieceType::PAWN, PieceType::KING)};
constexpr EvalTerm knightOutposts = {PieceSet(PieceType::PAWN, PieceType::KNIGHT)};
constexpr EvalTerm bishopPawns = {PieceSet(PieceType::PAWN, PieceType::BISHOP)};
constexpr EvalTerm rookOpen = {PieceSet(PieceType::PAWN, PieceType::ROOK)};


}

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
PackedScore evaluatePassedPawns(const Board& board, const PawnStructure& pawnStructure)
{
    constexpr Color them = ~us;
    Square ourKing = board.kingSq(us);
    Square theirKing = board.kingSq(them);

    Bitboard passers = pawnStructure.passedPawns & board.pieces(us);

    PackedScore eval{0, 0};

    while (passers.any())
    {
        Square passer = passers.poplsb();
        int rank = passer.relativeRank<us>();
        if (rank >= RANK_4)
        {
            eval += OUR_PASSER_PROXIMITY[Square::chebyshev(ourKing, passer)];
            eval += THEIR_PASSER_PROXIMITY[Square::chebyshev(theirKing, passer)];

            if (board.pieceAt(passer + attacks::pawnPushOffset<us>()) == Piece::NONE)
                eval += FREE_PASSER[rank];
        }
    }

    return eval;
}

template<Color us>
PackedScore evalKingPawnFile(uint32_t file, Bitboard ourPawns, Bitboard theirPawns, Square theirKing)
{
    PackedScore eval{0, 0};
    uint32_t kingFile = theirKing.file();
    {
        Bitboard filePawns = ourPawns & Bitboard::fileBB(file);
        // 4 = e file
        int idx = (kingFile == file) ? 1 : (kingFile >= 4) == (kingFile < file) ? 0 : 2;

        int rankDist = filePawns.any() ?
            std::abs((us == Color::WHITE ? filePawns.msb() : filePawns.lsb()).rank() - theirKing.rank()) :
            7;
        eval += PAWN_STORM[idx][rankDist];
    }
    {
        Bitboard filePawns = theirPawns & Bitboard::fileBB(file);
        // 4 = e file
        int idx = (kingFile == file) ? 1 : (kingFile >= 4) == (kingFile < file) ? 0 : 2;
        int rankDist = filePawns.any() ?
            std::abs((us == Color::WHITE ? filePawns.msb() : filePawns.lsb()).rank() - theirKing.rank()) :
            7;
        eval += PAWN_SHIELD[idx][rankDist];
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

    for (uint32_t file = 0; file < 8; file++)
        eval += evalKingPawnFile<us>(file, ourPawns, theirPawns, theirKing);

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


}

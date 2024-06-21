#include "eval.h"
#include "../attacks.h"

namespace eval
{

template<typename T>
struct ColorArray: public std::array<T, 2>
{
    using std::array<T, 2>::operator[];

    T& operator[](Color p)
    {
        return (*this)[static_cast<int>(p)];
    }

    const T& operator[](Color p) const
    {
        return (*this)[static_cast<int>(p)];
    }
};

template<typename T>
struct PieceTypeArray : public std::array<T, 6>
{
    using std::array<T, 6>::operator[];

    T& operator[](PieceType p)
    {
        return (*this)[static_cast<int>(p)];
    }

    const T& operator[](PieceType p) const
    {
        return (*this)[static_cast<int>(p)];
    }
};

struct EvalData
{
    ColorArray<Bitboard> mobilityArea;
    ColorArray<Bitboard> attacked;
    ColorArray<Bitboard> attackedBy2;
    ColorArray<PieceTypeArray<Bitboard>> attackedBy;
    ColorArray<Bitboard> pawnAttackSpans;
    ColorArray<Bitboard> kingRing;
    ColorArray<PackedScore> attackWeight;
    ColorArray<int> attackCount;
};

template<Color us, PieceType piece>
PackedScore evaluatePieces(const Board& board, EvalData& evalData)
{
    constexpr Color them = ~us;
    Bitboard ourPawns = board.getPieces(us, PieceType::PAWN);
    Bitboard theirPawns = board.getPieces(them, PieceType::PAWN);

    PackedScore eval{0, 0};
    Bitboard pieces = board.getPieces(us, piece);
    if constexpr (piece == PieceType::BISHOP)
        if (pieces.multiple())
            eval += BISHOP_PAIR;

    Bitboard outpostSquares = RANK_4 | RANK_5 | (us == Color::WHITE ? RANK_6 : RANK_3);

    while (pieces)
    {
        uint32_t sq = pieces.poplsb();
        if ((Bitboard::fromSquare(sq) & evalData.attackedBy[us][PieceType::PAWN]).any())
            eval += PAWN_DEFENDED_PIECE[static_cast<int>(piece) - static_cast<int>(PieceType::KNIGHT)];

        Bitboard attacks = attacks::pieceAttacks<piece>(sq, board.getAllPieces());
        evalData.attackedBy[us][piece] |= attacks;
        evalData.attackedBy2[us] |= evalData.attacked[us] & attacks;
        evalData.attacked[us] |= attacks;

        eval += MOBILITY[static_cast<int>(piece) - static_cast<int>(PieceType::KNIGHT)][(attacks & evalData.mobilityArea[us]).popcount()];

        Bitboard threats = attacks & board.getColor(them);
        while (threats)
            eval += THREATS[static_cast<int>(piece)][static_cast<int>(getPieceType(board.getPieceAt(threats.poplsb())))];

        if (Bitboard kingRingAtks = evalData.kingRing[them] & attacks; kingRingAtks.any())
        {
            evalData.attackWeight[us] += KING_ATTACKER_WEIGHT[static_cast<int>(piece) - static_cast<int>(PieceType::KNIGHT)];
            evalData.attackCount[us] += kingRingAtks.popcount();
        }

        Bitboard fileBB = Bitboard::fileBB(fileOf(sq));

        if constexpr (piece == PieceType::ROOK)
        {
            if ((ourPawns & fileBB).empty())
                eval += (theirPawns & fileBB).any() ? ROOK_OPEN[1] : ROOK_OPEN[0];
        }

        if constexpr (piece == PieceType::KNIGHT)
        {
            Bitboard outposts = outpostSquares & ~evalData.pawnAttackSpans[them] & evalData.attackedBy[us][PieceType::PAWN];
            if (Bitboard::fromSquare(sq) & outposts)
                eval += KNIGHT_OUTPOST;
        }
    }

    return eval;
}



template<Color us>
PackedScore evaluatePawns(const Board& board)
{
    Bitboard ourPawns = board.getPieces(us, PieceType::PAWN);

    PackedScore eval{0, 0};

    Bitboard pawns = ourPawns;
    while (pawns)
    {
        uint32_t sq = pawns.poplsb();
        if (board.isPassedPawn(sq))
            eval += PASSED_PAWN[relativeRankOf<us>(sq)];
        if (board.isIsolatedPawn(sq))
            eval += ISOLATED_PAWN[fileOf(sq)];
    }

    Bitboard phalanx = ourPawns & ourPawns.west();
    while (phalanx)
        eval += PAWN_PHALANX[relativeRankOf<us>(phalanx.poplsb())];

    Bitboard defended = ourPawns & attacks::pawnAttacks<us>(ourPawns);
    while (defended)
        eval += DEFENDED_PAWN[relativeRankOf<us>(defended.poplsb())];

    return eval;
}

PackedScore evaluatePawns(const Board& board, PawnTable* pawnTable)
{
    if (pawnTable)
    {
        const auto& entry = pawnTable->probe(board.pawnKey());
        if (entry.pawnKey == board.pawnKey())
            return entry.score;
    }

    PackedScore eval = evaluatePawns<Color::WHITE>(board) - evaluatePawns<Color::BLACK>(board);
    if (pawnTable)
    {
        PawnEntry replace = {board.pawnKey(), eval};
        pawnTable->store(replace);
    }
    return eval;
}

// I'll figure out how to add the other pieces here later
template<Color us>
PackedScore evaluateThreats(const Board& board, const EvalData& evalData)
{
    constexpr Color them = ~us;
    Bitboard threats = evalData.attackedBy[us][PieceType::PAWN] & board.getColor(them);
    PackedScore eval{0, 0};
    while (threats)
        eval += THREATS[static_cast<int>(PieceType::PAWN)][static_cast<int>(getPieceType(board.getPieceAt(threats.poplsb())))];
    return eval;
}

template<Color us>
PackedScore evalKingPawnFile(uint32_t file, Bitboard ourPawns, Bitboard theirPawns, uint32_t theirKing)
{
    PackedScore eval{0, 0};
    uint32_t kingFile = fileOf(theirKing);
    {
        Bitboard filePawns = ourPawns & Bitboard::fileBB(file);
        // 4 = e file
        int idx = (kingFile == file) ? 1 : (kingFile >= 4) == (kingFile < file) ? 0 : 2;

        int rankDist = filePawns ?
            std::abs(rankOf(us == Color::WHITE ? filePawns.msb() : filePawns.lsb()) - rankOf(theirKing)) :
            7;
        eval += PAWN_STORM[idx][rankDist];
    }
    {
        Bitboard filePawns = theirPawns & Bitboard::fileBB(file);
        // 4 = e file
        int idx = (kingFile == file) ? 1 : (kingFile >= 4) == (kingFile < file) ? 0 : 2;
        int rankDist = filePawns ?
            std::abs(rankOf(us == Color::WHITE ? filePawns.msb() : filePawns.lsb()) - rankOf(theirKing)) :
            7;
        eval += PAWN_SHIELD[idx][rankDist];
    }
    return eval;
}

template<Color us>
PackedScore evaluateKings(const Board& board, const EvalData& evalData)
{
    constexpr Color them = ~us;
    Bitboard ourPawns = board.getPieces(us, PieceType::PAWN);
    Bitboard theirPawns = board.getPieces(them, PieceType::PAWN);

    uint32_t theirKing = board.getPieces(them, PieceType::KING).lsb();

    PackedScore eval{0, 0};

    for (uint32_t file = 0; file < 8; file++)
        eval += evalKingPawnFile<us>(file, ourPawns, theirPawns, theirKing);

    Bitboard rookCheckSquares = attacks::rookAttacks(theirKing, board.getAllPieces());
    Bitboard bishopCheckSquares = attacks::rookAttacks(theirKing, board.getAllPieces());

    Bitboard knightChecks = evalData.attackedBy[us][PieceType::KNIGHT] & attacks::knightAttacks(theirKing);
    Bitboard bishopChecks = evalData.attackedBy[us][PieceType::BISHOP] & bishopCheckSquares;
    Bitboard rookChecks = evalData.attackedBy[us][PieceType::ROOK] & rookCheckSquares;
    Bitboard queenChecks = evalData.attackedBy[us][PieceType::QUEEN] & (bishopCheckSquares | rookCheckSquares);

    Bitboard safe = ~evalData.attacked[them] | (~evalData.attackedBy2[them] & evalData.attackedBy[them][PieceType::KING]);

    eval += SAFE_KNIGHT_CHECK * (knightChecks & safe).popcount();
    eval += SAFE_BISHOP_CHECK * (bishopChecks & safe).popcount();
    eval += SAFE_ROOK_CHECK * (rookChecks & safe).popcount();
    eval += SAFE_QUEEN_CHECK * (queenChecks & safe).popcount();

    eval += evalData.attackWeight[us];
    int attackCount = std::min(evalData.attackCount[us], 13);
    eval += KING_ATTACKS[attackCount];

    return eval;
}

void initEvalData(const Board& board, EvalData& evalData)
{
    Bitboard whitePawns = board.getPieces(Color::WHITE, PieceType::PAWN);
    Bitboard blackPawns = board.getPieces(Color::BLACK, PieceType::PAWN);
    Bitboard whitePawnAttacks = attacks::pawnAttacks<Color::WHITE>(whitePawns);
    Bitboard blackPawnAttacks = attacks::pawnAttacks<Color::BLACK>(blackPawns);
    uint32_t whiteKing = board.getPieces(Color::WHITE, PieceType::KING).lsb();
    uint32_t blackKing = board.getPieces(Color::BLACK, PieceType::KING).lsb();

    evalData.mobilityArea[Color::WHITE] = ~blackPawnAttacks;
    evalData.pawnAttackSpans[Color::WHITE] = attacks::fillUp<Color::WHITE>(whitePawnAttacks);
    evalData.attacked[Color::WHITE] = evalData.attackedBy[Color::WHITE][PieceType::PAWN] = whitePawnAttacks;

    Bitboard whiteKingAtks = attacks::kingAttacks(whiteKing);
    evalData.attackedBy[Color::WHITE][PieceType::KING] = whiteKingAtks;
    evalData.attackedBy2[Color::WHITE] = evalData.attacked[Color::WHITE] & whiteKingAtks;
    evalData.attacked[Color::WHITE] |= whiteKingAtks;
    evalData.kingRing[Color::WHITE] = (whiteKingAtks | whiteKingAtks.north()) & ~Bitboard::fromSquare(whiteKing);

    evalData.mobilityArea[Color::BLACK] = ~whitePawnAttacks;
    evalData.pawnAttackSpans[Color::BLACK] = attacks::fillUp<Color::BLACK>(blackPawnAttacks);
    evalData.attacked[Color::BLACK] = evalData.attackedBy[Color::BLACK][PieceType::PAWN] = blackPawnAttacks;

    Bitboard blackKingAtks = attacks::kingAttacks(blackKing);
    evalData.attackedBy[Color::BLACK][PieceType::KING] = blackKingAtks;
    evalData.attackedBy2[Color::BLACK] = evalData.attacked[Color::BLACK] & blackKingAtks;
    evalData.attacked[Color::BLACK] |= blackKingAtks;
    evalData.kingRing[Color::BLACK] = (blackKingAtks | blackKingAtks.south()) & ~Bitboard::fromSquare(blackKing);
}

int evaluate(const Board& board, search::SearchThread* thread)
{
    EvalData evalData = {};
    initEvalData(board, evalData);

    if (!eval::canForceMate(board))
        return SCORE_DRAW;

    Color color = board.sideToMove();
    PackedScore eval = board.evalState().materialPsqt;

    eval += evaluatePieces<Color::WHITE, PieceType::KNIGHT>(board, evalData) - evaluatePieces<Color::BLACK, PieceType::KNIGHT>(board, evalData);
    eval += evaluatePieces<Color::WHITE, PieceType::BISHOP>(board, evalData) - evaluatePieces<Color::BLACK, PieceType::BISHOP>(board, evalData);
    eval += evaluatePieces<Color::WHITE, PieceType::ROOK>(board, evalData) - evaluatePieces<Color::BLACK, PieceType::ROOK>(board, evalData);
    eval += evaluatePieces<Color::WHITE, PieceType::QUEEN>(board, evalData) - evaluatePieces<Color::BLACK, PieceType::QUEEN>(board, evalData);

    eval += evaluateKings<Color::WHITE>(board, evalData) - evaluateKings<Color::BLACK>(board, evalData);

    eval += evaluatePawns(board, thread ? &thread->pawnTable : nullptr);
    eval += evaluateThreats<Color::WHITE>(board, evalData) - evaluateThreats<Color::BLACK>(board, evalData);


    return (color == Color::WHITE ? 1 : -1) * eval::getFullEval(eval.mg(), eval.eg(), board.evalState().phase);
}


}

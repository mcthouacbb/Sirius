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

    Bitboard passedPawns;
};

template<Color us, PieceType piece>
PackedScore evaluatePieces(const Board& board, EvalData& evalData)
{
    constexpr Color them = ~us;
    Bitboard ourPawns = board.pieces(us, PieceType::PAWN);
    Bitboard theirPawns = board.pieces(them, PieceType::PAWN);

    PackedScore eval{0, 0};
    Bitboard pieces = board.pieces(us, piece);
    if constexpr (piece == PieceType::BISHOP)
        if (pieces.multiple())
            eval += BISHOP_PAIR;

    Bitboard occupancy = board.allPieces();
    if constexpr (piece == PieceType::BISHOP)
        occupancy ^= board.pieces(us, PieceType::BISHOP) | board.pieces(us, PieceType::QUEEN);
    else if constexpr (piece == PieceType::ROOK)
        occupancy ^= board.pieces(us, PieceType::ROOK) | board.pieces(us, PieceType::QUEEN);
    else if constexpr (piece == PieceType::QUEEN)
        occupancy ^= board.pieces(us, PieceType::BISHOP) | board.pieces(us, PieceType::ROOK);

    Bitboard outpostSquares = RANK_4 | RANK_5 | (us == Color::WHITE ? RANK_6 : RANK_3);

    while (pieces.any())
    {
        uint32_t sq = pieces.poplsb();
        Bitboard attacks = attacks::pieceAttacks<piece>(sq, occupancy);
        if ((board.checkBlockers(us) & Bitboard::fromSquare(sq)).any())
            attacks &= attacks::inBetweenSquares(sq, board.kingSq(us));

        evalData.attackedBy[us][piece] |= attacks;
        evalData.attackedBy2[us] |= evalData.attacked[us] & attacks;
        evalData.attacked[us] |= attacks;

        eval += MOBILITY[static_cast<int>(piece) - static_cast<int>(PieceType::KNIGHT)][(attacks & evalData.mobilityArea[us]).popcount()];

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
            if ((Bitboard::fromSquare(sq) & outposts).any())
                eval += KNIGHT_OUTPOST;
        }

        if constexpr (piece == PieceType::BISHOP)
        {
            bool lightSquare = (Bitboard::fromSquare(sq) & LIGHT_SQUARES).any();
            Bitboard sameColorPawns = board.pieces(us, PieceType::PAWN) & (lightSquare ? LIGHT_SQUARES : DARK_SQUARES);
            eval += BISHOP_PAWNS[std::min(sameColorPawns.popcount(), 6u)];
        }
    }

    return eval;
}



template<Color us>
PackedScore evaluatePawns(const Board& board, EvalData& evalData)
{
    Bitboard ourPawns = board.pieces(us, PieceType::PAWN);

    PackedScore eval{0, 0};

    Bitboard pawns = ourPawns;
    while (pawns.any())
    {
        uint32_t sq = pawns.poplsb();
        if (board.isPassedPawn(sq))
        {
            evalData.passedPawns |= Bitboard::fromSquare(sq);
            eval += PASSED_PAWN[relativeRankOf<us>(sq)];
        }
        if (board.isIsolatedPawn(sq))
            eval += ISOLATED_PAWN[fileOf(sq)];
    }

    Bitboard phalanx = ourPawns & ourPawns.west();
    while (phalanx.any())
        eval += PAWN_PHALANX[relativeRankOf<us>(phalanx.poplsb())];

    Bitboard defended = ourPawns & attacks::pawnAttacks<us>(ourPawns);
    while (defended.any())
        eval += DEFENDED_PAWN[relativeRankOf<us>(defended.poplsb())];

    return eval;
}

PackedScore evaluatePawns(const Board& board, EvalData& evalData, PawnTable* pawnTable)
{
    if (pawnTable)
    {
        const auto& entry = pawnTable->probe(board.pawnKey());
        if (entry.pawnKey == board.pawnKey())
        {
            evalData.passedPawns = entry.passedPawns;
            return entry.score;
        }
    }

    PackedScore eval = evaluatePawns<Color::WHITE>(board, evalData) - evaluatePawns<Color::BLACK>(board, evalData);
    if (pawnTable)
    {
        PawnEntry replace = {board.pawnKey(), evalData.passedPawns, eval};
        pawnTable->store(replace);
    }

    return eval;
}

template<Color us>
PackedScore evaluateKingPawn(const Board& board, const EvalData& evalData)
{
    constexpr Color them = ~us;
    uint32_t ourKing = board.kingSq(us);
    uint32_t theirKing = board.kingSq(them);

    Bitboard passers = evalData.passedPawns & board.pieces(us);

    PackedScore eval{0, 0};

    while (passers.any())
    {
        uint32_t passer = passers.poplsb();
        eval += OUR_PASSER_PROXIMITY[chebyshev(ourKing, passer)];
        eval += THEIR_PASSER_PROXIMITY[chebyshev(theirKing, passer)];
    }

    return eval;
}

// I'll figure out how to add the other pieces here later
template<Color us>
PackedScore evaluateThreats(const Board& board, const EvalData& evalData)
{
    constexpr Color them = ~us;

    PackedScore eval{0, 0};

    Bitboard defendedBB = evalData.attacked[them];

    Bitboard pawnThreats = evalData.attackedBy[us][PieceType::PAWN] & board.pieces(them);
    while (pawnThreats.any())
    {
        PieceType threatened = getPieceType(board.pieceAt(pawnThreats.poplsb()));
        eval += THREAT_BY_PAWN[static_cast<int>(threatened)];
    }

    Bitboard knightThreats = evalData.attackedBy[us][PieceType::KNIGHT] & board.pieces(them);
    while (knightThreats.any())
    {
        int threat = knightThreats.poplsb();
        PieceType threatened = getPieceType(board.pieceAt(threat));
        bool defended = (defendedBB & Bitboard::fromSquare(threat)).any();
        eval += THREAT_BY_KNIGHT[defended][static_cast<int>(threatened)];
    }

    Bitboard bishopThreats = evalData.attackedBy[us][PieceType::BISHOP] & board.pieces(them);
    while (bishopThreats.any())
    {
        int threat = bishopThreats.poplsb();
        PieceType threatened = getPieceType(board.pieceAt(threat));
        bool defended = (defendedBB & Bitboard::fromSquare(threat)).any();
        eval += THREAT_BY_BISHOP[defended][static_cast<int>(threatened)];
    }

    Bitboard rookThreats = evalData.attackedBy[us][PieceType::ROOK] & board.pieces(them);
    while (rookThreats.any())
    {
        int threat = rookThreats.poplsb();
        PieceType threatened = getPieceType(board.pieceAt(threat));
        bool defended = (defendedBB & Bitboard::fromSquare(threat)).any();
        eval += THREAT_BY_ROOK[defended][static_cast<int>(threatened)];
    }

    Bitboard queenThreats = evalData.attackedBy[us][PieceType::QUEEN] & board.pieces(them);
    while (queenThreats.any())
    {
        int threat = queenThreats.poplsb();
        PieceType threatened = getPieceType(board.pieceAt(threat));
        bool defended = (defendedBB & Bitboard::fromSquare(threat)).any();
        eval += THREAT_BY_QUEEN[defended][static_cast<int>(threatened)];
    }

    Bitboard kingThreats = evalData.attackedBy[us][PieceType::KING] & board.pieces(them) & ~defendedBB;
    while (kingThreats.any())
    {
        PieceType threatened = getPieceType(board.pieceAt(kingThreats.poplsb()));
        eval += THREAT_BY_KING[static_cast<int>(threatened)];
    }

    Bitboard nonPawnEnemies = board.pieces(them) & ~board.pieces(PieceType::PAWN);

    Bitboard safe = ~defendedBB | (evalData.attacked[us] & ~evalData.attackedBy[them][PieceType::PAWN]);
    Bitboard pushes = attacks::pawnPushes<us>(board.pieces(us, PieceType::PAWN)) & ~board.allPieces();
    pushes |= attacks::pawnPushes<us>(pushes & Bitboard::nthRank<us, 2>()) & ~board.allPieces();

    Bitboard pushThreats = attacks::pawnAttacks<us>(pushes & safe) & nonPawnEnemies;
    eval += PUSH_THREAT * pushThreats.popcount();
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

        int rankDist = filePawns.any() ?
            std::abs(rankOf(us == Color::WHITE ? filePawns.msb() : filePawns.lsb()) - rankOf(theirKing)) :
            7;
        eval += PAWN_STORM[idx][rankDist];
    }
    {
        Bitboard filePawns = theirPawns & Bitboard::fileBB(file);
        // 4 = e file
        int idx = (kingFile == file) ? 1 : (kingFile >= 4) == (kingFile < file) ? 0 : 2;
        int rankDist = filePawns.any() ?
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
    Bitboard ourPawns = board.pieces(us, PieceType::PAWN);
    Bitboard theirPawns = board.pieces(them, PieceType::PAWN);

    uint32_t theirKing = board.pieces(them, PieceType::KING).lsb();

    PackedScore eval{0, 0};

    for (uint32_t file = 0; file < 8; file++)
        eval += evalKingPawnFile<us>(file, ourPawns, theirPawns, theirKing);

    Bitboard rookCheckSquares = attacks::rookAttacks(theirKing, board.allPieces());
    Bitboard bishopCheckSquares = attacks::rookAttacks(theirKing, board.allPieces());

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
    Bitboard whitePawns = board.pieces(Color::WHITE, PieceType::PAWN);
    Bitboard blackPawns = board.pieces(Color::BLACK, PieceType::PAWN);
    Bitboard whitePawnAttacks = attacks::pawnAttacks<Color::WHITE>(whitePawns);
    Bitboard blackPawnAttacks = attacks::pawnAttacks<Color::BLACK>(blackPawns);
    uint32_t whiteKing = board.kingSq(Color::WHITE);
    uint32_t blackKing = board.kingSq(Color::BLACK);

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


int evaluateScale(const Board& board, PackedScore eval)
{
    Color strongSide = eval.eg() > 0 ? Color::WHITE : Color::BLACK;

    int strongPawns = board.pieces(strongSide, PieceType::PAWN).popcount();

    return 80 + strongPawns * 7;
}

int evaluate(const Board& board, search::SearchThread* thread)
{
    constexpr int SCALE_FACTOR = 128;

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

    eval += evaluatePawns(board, evalData, thread ? &thread->pawnTable : nullptr);
    eval += evaluateKingPawn<Color::WHITE>(board, evalData) - evaluateKingPawn<Color::BLACK>(board, evalData);
    eval += evaluateThreats<Color::WHITE>(board, evalData) - evaluateThreats<Color::BLACK>(board, evalData);

    int scale = evaluateScale(board, eval);

    eval += (color == Color::WHITE ? TEMPO : -TEMPO);

    return (color == Color::WHITE ? 1 : -1) * eval::getFullEval(eval.mg(), eval.eg() * scale / SCALE_FACTOR, board.evalState().phase);
}


}

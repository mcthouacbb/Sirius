#include "eval.h"
#include "../attacks.h"
#include "../util/color_piece_array.h"
#include "pawn_structure.h"

namespace eval
{

struct EvalData
{
    ColorArray<Bitboard> mobilityArea;
    ColorArray<Bitboard> attacked;
    ColorArray<Bitboard> attackedBy2;
    ColorArray<PieceTypeArray<Bitboard>> attackedBy;
    ColorArray<Bitboard> kingRing;
    ColorArray<PackedScore> attackWeight;
    ColorArray<int> attackCount;
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


    while (pieces.any())
    {
        Square sq = pieces.poplsb();
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
        Square threat = knightThreats.poplsb();
        PieceType threatened = getPieceType(board.pieceAt(threat));
        bool defended = (defendedBB & Bitboard::fromSquare(threat)).any();
        eval += THREAT_BY_KNIGHT[defended][static_cast<int>(threatened)];
    }

    Bitboard bishopThreats = evalData.attackedBy[us][PieceType::BISHOP] & board.pieces(them);
    while (bishopThreats.any())
    {
        Square threat = bishopThreats.poplsb();
        PieceType threatened = getPieceType(board.pieceAt(threat));
        bool defended = (defendedBB & Bitboard::fromSquare(threat)).any();
        eval += THREAT_BY_BISHOP[defended][static_cast<int>(threatened)];
    }

    Bitboard rookThreats = evalData.attackedBy[us][PieceType::ROOK] & board.pieces(them);
    while (rookThreats.any())
    {
        Square threat = rookThreats.poplsb();
        PieceType threatened = getPieceType(board.pieceAt(threat));
        bool defended = (defendedBB & Bitboard::fromSquare(threat)).any();
        eval += THREAT_BY_ROOK[defended][static_cast<int>(threatened)];
    }

    Bitboard queenThreats = evalData.attackedBy[us][PieceType::QUEEN] & board.pieces(them);
    while (queenThreats.any())
    {
        Square threat = queenThreats.poplsb();
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
    pushes |= attacks::pawnPushes<us>(pushes & Bitboard::nthRank<us, RANK_3>()) & ~board.allPieces();

    Bitboard pushThreats = attacks::pawnAttacks<us>(pushes & safe) & nonPawnEnemies;
    eval += PUSH_THREAT * pushThreats.popcount();
    return eval;
}

template<Color us>
PackedScore evaluateKings(const Board& board, const EvalData& evalData)
{
    constexpr Color them = ~us;

    Square theirKing = board.kingSq(them);

    PackedScore eval{0, 0};

    Bitboard rookCheckSquares = attacks::rookAttacks(theirKing, board.allPieces());
    Bitboard bishopCheckSquares = attacks::bishopAttacks(theirKing, board.allPieces());

    Bitboard knightChecks = evalData.attackedBy[us][PieceType::KNIGHT] & attacks::knightAttacks(theirKing);
    Bitboard bishopChecks = evalData.attackedBy[us][PieceType::BISHOP] & bishopCheckSquares;
    Bitboard rookChecks = evalData.attackedBy[us][PieceType::ROOK] & rookCheckSquares;
    Bitboard queenChecks = evalData.attackedBy[us][PieceType::QUEEN] & (bishopCheckSquares | rookCheckSquares);

    Bitboard safe = ~evalData.attacked[them] | (~evalData.attackedBy2[them] & evalData.attackedBy[them][PieceType::KING]);
    Bitboard weak = safe & evalData.attacked[us];

    eval += SAFE_KNIGHT_CHECK * (knightChecks & safe).popcount();
    eval += SAFE_BISHOP_CHECK * (bishopChecks & safe).popcount();
    eval += SAFE_ROOK_CHECK * (rookChecks & safe).popcount();
    eval += SAFE_QUEEN_CHECK * (queenChecks & safe).popcount();

    eval += evalData.attackWeight[us];
    int attackCount = std::min(evalData.attackCount[us], 13);
    eval += KING_ATTACKS[attackCount];
    eval += WEAK_KING_RING[std::min((weak & evalData.kingRing[them]).popcount(), 8u)];

    return eval;
}

template<Color us>
PackedScore evaluatePassedPawns(const Board& board, const PawnStructure& pawnStructure, const EvalData& evalData)
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
            Square pushSq = passer + attacks::pawnPushOffset<us>();

            bool blocked = board.pieceAt(pushSq) != Piece::NONE;
            bool controlled = (evalData.attacked[them] & Bitboard::fromSquare(pushSq)).any();
            eval += PASSED_PAWN[blocked][controlled][rank];

            eval += OUR_PASSER_PROXIMITY[Square::chebyshev(ourKing, passer)];
            eval += THEIR_PASSER_PROXIMITY[Square::chebyshev(theirKing, passer)];
        }
    }

    return eval;
}

void initEvalData(const Board& board, EvalData& evalData, const PawnStructure& pawnStructure)
{
    Bitboard whitePawns = board.pieces(Color::WHITE, PieceType::PAWN);
    Bitboard blackPawns = board.pieces(Color::BLACK, PieceType::PAWN);
    Square whiteKing = board.kingSq(Color::WHITE);
    Square blackKing = board.kingSq(Color::BLACK);

    evalData.mobilityArea[Color::WHITE] = ~pawnStructure.pawnAttacks[Color::BLACK];
    evalData.attacked[Color::WHITE] = evalData.attackedBy[Color::WHITE][PieceType::PAWN] = pawnStructure.pawnAttacks[Color::WHITE];

    Bitboard whiteKingAtks = attacks::kingAttacks(whiteKing);
    evalData.attackedBy[Color::WHITE][PieceType::KING] = whiteKingAtks;
    evalData.attackedBy2[Color::WHITE] = evalData.attacked[Color::WHITE] & whiteKingAtks;
    evalData.attacked[Color::WHITE] |= whiteKingAtks;
    evalData.kingRing[Color::WHITE] = (whiteKingAtks | whiteKingAtks.north()) & ~Bitboard::fromSquare(whiteKing);

    evalData.mobilityArea[Color::BLACK] = ~pawnStructure.pawnAttacks[Color::WHITE];
    evalData.attacked[Color::BLACK] = evalData.attackedBy[Color::BLACK][PieceType::PAWN] = pawnStructure.pawnAttacks[Color::BLACK];

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
    if (!eval::canForceMate(board))
        return SCORE_DRAW;

    constexpr int SCALE_FACTOR = 128;

    Color color = board.sideToMove();
    PackedScore eval = thread->evalState.score(board);

    const PawnStructure& pawnStructure = thread->evalState.pawnStructure();

    EvalData evalData = {};
    initEvalData(board, evalData, pawnStructure);

    eval += evaluatePieces<Color::WHITE, PieceType::KNIGHT>(board, evalData) - evaluatePieces<Color::BLACK, PieceType::KNIGHT>(board, evalData);
    eval += evaluatePieces<Color::WHITE, PieceType::BISHOP>(board, evalData) - evaluatePieces<Color::BLACK, PieceType::BISHOP>(board, evalData);
    eval += evaluatePieces<Color::WHITE, PieceType::ROOK>(board, evalData) - evaluatePieces<Color::BLACK, PieceType::ROOK>(board, evalData);
    eval += evaluatePieces<Color::WHITE, PieceType::QUEEN>(board, evalData) - evaluatePieces<Color::BLACK, PieceType::QUEEN>(board, evalData);

    eval += evaluateKings<Color::WHITE>(board, evalData) - evaluateKings<Color::BLACK>(board, evalData);
    eval += evaluatePassedPawns<Color::WHITE>(board, pawnStructure, evalData) - evaluatePassedPawns<Color::BLACK>(board, pawnStructure, evalData);
    eval += evaluateThreats<Color::WHITE>(board, evalData) - evaluateThreats<Color::BLACK>(board, evalData);

    int scale = evaluateScale(board, eval);

    eval += (color == Color::WHITE ? TEMPO : -TEMPO);

    return (color == Color::WHITE ? 1 : -1) * eval::getFullEval(eval.mg(), eval.eg() * scale / SCALE_FACTOR, thread->evalState.phase());
}


}

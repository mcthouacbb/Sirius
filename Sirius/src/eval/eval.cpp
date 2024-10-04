#include "eval.h"
#include "../attacks.h"
#include "../util/enum_array.h"
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

using enum PieceType;
using enum Color;

template<Color us, PieceType piece>
PackedScore evaluatePieces(const Board& board, EvalData& evalData)
{
    constexpr Color them = ~us;

    PackedScore eval{0, 0};
    Bitboard pieces = board.pieces(us, piece);
    if constexpr (piece == BISHOP)
        if (pieces.multiple())
            eval += BISHOP_PAIR;

    Bitboard occupancy = board.allPieces();
    if constexpr (piece == BISHOP)
        occupancy ^= board.pieces(us, BISHOP) | board.pieces(us, QUEEN);
    else if constexpr (piece == ROOK)
        occupancy ^= board.pieces(us, ROOK) | board.pieces(us, QUEEN);
    else if constexpr (piece == QUEEN)
        occupancy ^= board.pieces(us, BISHOP) | board.pieces(us, ROOK);


    while (pieces.any())
    {
        Square sq = pieces.poplsb();
        Bitboard attacks = attacks::pieceAttacks<piece>(sq, occupancy);
        if ((board.checkBlockers(us) & Bitboard::fromSquare(sq)).any())
            attacks &= attacks::inBetweenSquares(sq, board.kingSq(us));

        evalData.attackedBy[us][piece] |= attacks;
        evalData.attackedBy2[us] |= evalData.attacked[us] & attacks;
        evalData.attacked[us] |= attacks;

        eval += MOBILITY[static_cast<int>(piece) - static_cast<int>(KNIGHT)][(attacks & evalData.mobilityArea[us]).popcount()];

        if (Bitboard kingRingAtks = evalData.kingRing[them] & attacks; kingRingAtks.any())
        {
            evalData.attackWeight[us] += KING_ATTACKER_WEIGHT[static_cast<int>(piece) - static_cast<int>(KNIGHT)];
            evalData.attackCount[us] += kingRingAtks.popcount();
        }
    }

    return eval;
}

template<Color us>
PackedScore evaluateThreats(const Board& board, const EvalData& evalData)
{
    constexpr Color them = ~us;

    PackedScore eval{0, 0};

    Bitboard defendedBB =
        evalData.attackedBy2[them] |
        evalData.attackedBy[them][PAWN] |
        (evalData.attacked[them] & ~evalData.attackedBy2[us]);

    Bitboard pawnThreats = evalData.attackedBy[us][PAWN] & board.pieces(them);
    while (pawnThreats.any())
    {
        PieceType threatened = getPieceType(board.pieceAt(pawnThreats.poplsb()));
        eval += THREAT_BY_PAWN[static_cast<int>(threatened)];
    }

    Bitboard knightThreats = evalData.attackedBy[us][KNIGHT] & board.pieces(them);
    while (knightThreats.any())
    {
        Square threat = knightThreats.poplsb();
        PieceType threatened = getPieceType(board.pieceAt(threat));
        bool defended = (defendedBB & Bitboard::fromSquare(threat)).any();
        eval += THREAT_BY_KNIGHT[defended][static_cast<int>(threatened)];
    }

    Bitboard bishopThreats = evalData.attackedBy[us][BISHOP] & board.pieces(them);
    while (bishopThreats.any())
    {
        Square threat = bishopThreats.poplsb();
        PieceType threatened = getPieceType(board.pieceAt(threat));
        bool defended = (defendedBB & Bitboard::fromSquare(threat)).any();
        eval += THREAT_BY_BISHOP[defended][static_cast<int>(threatened)];
    }

    Bitboard rookThreats = evalData.attackedBy[us][ROOK] & board.pieces(them);
    while (rookThreats.any())
    {
        Square threat = rookThreats.poplsb();
        PieceType threatened = getPieceType(board.pieceAt(threat));
        bool defended = (defendedBB & Bitboard::fromSquare(threat)).any();
        eval += THREAT_BY_ROOK[defended][static_cast<int>(threatened)];
    }

    Bitboard queenThreats = evalData.attackedBy[us][QUEEN] & board.pieces(them);
    while (queenThreats.any())
    {
        Square threat = queenThreats.poplsb();
        PieceType threatened = getPieceType(board.pieceAt(threat));
        bool defended = (defendedBB & Bitboard::fromSquare(threat)).any();
        eval += THREAT_BY_QUEEN[defended][static_cast<int>(threatened)];
    }

    Bitboard kingThreats = evalData.attackedBy[us][KING] & board.pieces(them) & ~defendedBB;
    while (kingThreats.any())
    {
        PieceType threatened = getPieceType(board.pieceAt(kingThreats.poplsb()));
        eval += THREAT_BY_KING[static_cast<int>(threatened)];
    }

    Bitboard nonPawnEnemies = board.pieces(them) & ~board.pieces(PAWN);

    Bitboard safe = ~defendedBB | (evalData.attacked[us] & ~evalData.attackedBy[them][PieceType::PAWN] & ~evalData.attackedBy2[them]);
    Bitboard pushes = attacks::pawnPushes<us>(board.pieces(us, PAWN)) & ~board.allPieces();
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

    Bitboard knightChecks = evalData.attackedBy[us][KNIGHT] & attacks::knightAttacks(theirKing);
    Bitboard bishopChecks = evalData.attackedBy[us][BISHOP] & bishopCheckSquares;
    Bitboard rookChecks = evalData.attackedBy[us][ROOK] & rookCheckSquares;
    Bitboard queenChecks = evalData.attackedBy[us][QUEEN] & (bishopCheckSquares | rookCheckSquares);

    Bitboard safe = ~evalData.attacked[them] | (~evalData.attackedBy2[them] & evalData.attackedBy[them][KING]);

    eval += SAFE_KNIGHT_CHECK * (knightChecks & safe).popcount();
    eval += SAFE_BISHOP_CHECK * (bishopChecks & safe).popcount();
    eval += SAFE_ROOK_CHECK * (rookChecks & safe).popcount();
    eval += SAFE_QUEEN_CHECK * (queenChecks & safe).popcount();

    eval += UNSAFE_KNIGHT_CHECK * (knightChecks & ~safe).popcount();
    eval += UNSAFE_BISHOP_CHECK * (bishopChecks & ~safe).popcount();
    eval += UNSAFE_ROOK_CHECK * (rookChecks & ~safe).popcount();
    eval += UNSAFE_QUEEN_CHECK * (queenChecks & ~safe).popcount();

    eval += evalData.attackWeight[us];
    int attackCount = std::min(evalData.attackCount[us], 13);
    eval += KING_ATTACKS[attackCount];

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

PackedScore evaluateComplexity(const Board& board, const PawnStructure& pawnStructure, PackedScore eval)
{
    constexpr Bitboard KING_SIDE = FILE_A_BB | FILE_B_BB | FILE_C_BB | FILE_D_BB;
    constexpr Bitboard QUEEN_SIDE = ~KING_SIDE;
    Bitboard pawns = board.pieces(PAWN);
    bool pawnsBothSides = (pawns & KING_SIDE).any() && (pawns & QUEEN_SIDE).any();
    bool pawnEndgame = board.allPieces() == (pawns | board.pieces(KING));

    PackedScore complexity =
        COMPLEXITY_PAWNS * pawns.popcount() +
        COMPLEXITY_PASSERS * pawnStructure.passedPawns.popcount() +
        COMPLEXITY_PAWNS_BOTH_SIDES * pawnsBothSides +
        COMPLEXITY_PAWN_ENDGAME * pawnEndgame +
        COMPLEXITY_OFFSET;

    int egSign = (eval.eg() > 0) - (eval.eg() < 0);

    int egComplexity = std::max(complexity.eg(), -std::abs(eval.eg()));

    return PackedScore(0, egSign * egComplexity);
}

int evaluateScale(const Board& board, PackedScore eval)
{
    Color strongSide = eval.eg() > 0 ? WHITE : BLACK;

    int strongPawns = board.pieces(strongSide, PAWN).popcount();

    return 80 + strongPawns * 7;
}

void initEvalData(const Board& board, EvalData& evalData, const PawnStructure& pawnStructure)
{
    Square whiteKing = board.kingSq(WHITE);
    Square blackKing = board.kingSq(BLACK);

    evalData.mobilityArea[WHITE] = ~pawnStructure.pawnAttacks[BLACK];
    evalData.attacked[WHITE] = evalData.attackedBy[WHITE][PAWN] = pawnStructure.pawnAttacks[WHITE];

    Bitboard whiteKingAtks = attacks::kingAttacks(whiteKing);
    evalData.attackedBy[WHITE][KING] = whiteKingAtks;
    evalData.attackedBy2[WHITE] = evalData.attacked[WHITE] & whiteKingAtks;
    evalData.attacked[WHITE] |= whiteKingAtks;
    evalData.kingRing[WHITE] = (whiteKingAtks | whiteKingAtks.north()) & ~Bitboard::fromSquare(whiteKing);

    evalData.mobilityArea[BLACK] = ~pawnStructure.pawnAttacks[WHITE];
    evalData.attacked[BLACK] = evalData.attackedBy[BLACK][PAWN] = pawnStructure.pawnAttacks[BLACK];

    Bitboard blackKingAtks = attacks::kingAttacks(blackKing);
    evalData.attackedBy[BLACK][KING] = blackKingAtks;
    evalData.attackedBy2[BLACK] = evalData.attacked[BLACK] & blackKingAtks;
    evalData.attacked[BLACK] |= blackKingAtks;
    evalData.kingRing[BLACK] = (blackKingAtks | blackKingAtks.south()) & ~Bitboard::fromSquare(blackKing);
}

void nonIncrementalEval(const Board& board, const PawnStructure& pawnStructure, EvalData& evalData, PackedScore& eval)
{
    eval += evaluatePieces<WHITE, KNIGHT>(board, evalData) - evaluatePieces<BLACK, KNIGHT>(board, evalData);
    eval += evaluatePieces<WHITE, BISHOP>(board, evalData) - evaluatePieces<BLACK, BISHOP>(board, evalData);
    eval += evaluatePieces<WHITE, ROOK>(board, evalData) - evaluatePieces<BLACK, ROOK>(board, evalData);
    eval += evaluatePieces<WHITE, QUEEN>(board, evalData) - evaluatePieces<BLACK, QUEEN>(board, evalData);

    eval += evaluateKings<WHITE>(board, evalData) - evaluateKings<BLACK>(board, evalData);
    eval += evaluatePassedPawns<WHITE>(board, pawnStructure, evalData) - evaluatePassedPawns<BLACK>(board, pawnStructure, evalData);
    eval += evaluateThreats<WHITE>(board, evalData) - evaluateThreats<BLACK>(board, evalData);
    eval += evaluateComplexity(board, pawnStructure, eval);
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

    nonIncrementalEval(board, pawnStructure, evalData, eval);

    int scale = evaluateScale(board, eval);

    eval += (color == WHITE ? TEMPO : -TEMPO);

    return (color == WHITE ? 1 : -1) * eval::getFullEval(eval.mg(), eval.eg() * scale / SCALE_FACTOR, thread->evalState.phase());
}

int evaluateSingle(const Board& board)
{
    if (!eval::canForceMate(board))
        return SCORE_DRAW;

    EvalState evalState;
    evalState.initSingle(board);

    constexpr int SCALE_FACTOR = 128;

    Color color = board.sideToMove();
    PackedScore eval = evalState.score(board);

    const PawnStructure& pawnStructure = evalState.pawnStructure();

    EvalData evalData = {};
    initEvalData(board, evalData, pawnStructure);

    nonIncrementalEval(board, pawnStructure, evalData, eval);

    int scale = evaluateScale(board, eval);

    eval += (color == WHITE ? TEMPO : -TEMPO);

    return (color == WHITE ? 1 : -1) * eval::getFullEval(eval.mg(), eval.eg() * scale / SCALE_FACTOR, evalState.phase());
}


}

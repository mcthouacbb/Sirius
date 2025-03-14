#include "eval.h"
#include "../attacks.h"
#include "../util/enum_array.h"
#include "pawn_structure.h"
#include "endgame.h"

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
    ColorArray<int> attackerCount;
    ColorArray<int> attackCount;
};

using enum PieceType;
using enum Color;

template<Color us, PieceType piece>
PackedScore evaluatePieces(const Board& board, EvalData& evalData)
{
    constexpr Color them = ~us;
    constexpr Bitboard CENTER_SQUARES = (RANK_4_BB | RANK_5_BB) & (FILE_D_BB | FILE_E_BB);

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
        if (board.checkBlockers(us).has(sq))
            attacks &= attacks::inBetweenSquares(sq, board.kingSq(us));

        evalData.attackedBy[us][piece] |= attacks;
        evalData.attackedBy2[us] |= evalData.attacked[us] & attacks;
        evalData.attacked[us] |= attacks;

        eval += MOBILITY[static_cast<int>(piece) - static_cast<int>(KNIGHT)][(attacks & evalData.mobilityArea[us]).popcount()];

        if (Bitboard kingRingAtks = evalData.kingRing[them] & attacks; kingRingAtks.any())
        {
            evalData.attackWeight[us] += KING_ATTACKER_WEIGHT[static_cast<int>(piece) - static_cast<int>(KNIGHT)];
            evalData.attackCount[us] += kingRingAtks.popcount();
            evalData.attackerCount[us]++;
        }

        if (piece == BISHOP && (attacks & CENTER_SQUARES).multiple())
            eval += LONG_DIAG_BISHOP;
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
        bool defended = defendedBB.has(threat);
        eval += THREAT_BY_KNIGHT[defended][static_cast<int>(threatened)];
    }

    Bitboard bishopThreats = evalData.attackedBy[us][BISHOP] & board.pieces(them);
    while (bishopThreats.any())
    {
        Square threat = bishopThreats.poplsb();
        PieceType threatened = getPieceType(board.pieceAt(threat));
        bool defended = defendedBB.has(threat);
        eval += THREAT_BY_BISHOP[defended][static_cast<int>(threatened)];
    }

    Bitboard rookThreats = evalData.attackedBy[us][ROOK] & board.pieces(them);
    while (rookThreats.any())
    {
        Square threat = rookThreats.poplsb();
        PieceType threatened = getPieceType(board.pieceAt(threat));
        bool defended = defendedBB.has(threat);
        eval += THREAT_BY_ROOK[defended][static_cast<int>(threatened)];
    }

    Bitboard queenThreats = evalData.attackedBy[us][QUEEN] & board.pieces(them);
    while (queenThreats.any())
    {
        Square threat = queenThreats.poplsb();
        PieceType threatened = getPieceType(board.pieceAt(threat));
        bool defended = defendedBB.has(threat);
        eval += THREAT_BY_QUEEN[defended][static_cast<int>(threatened)];
    }

    Bitboard kingThreats = evalData.attackedBy[us][KING] & board.pieces(them) & ~defendedBB;
    while (kingThreats.any())
    {
        PieceType threatened = getPieceType(board.pieceAt(kingThreats.poplsb()));
        eval += THREAT_BY_KING[static_cast<int>(threatened)];
    }

    Bitboard nonPawnEnemies = board.pieces(them) & ~board.pieces(PAWN);

    Bitboard safe = ~defendedBB | (evalData.attacked[us] & ~evalData.attackedBy[them][PAWN] & ~evalData.attackedBy2[them]);
    Bitboard pushes = attacks::pawnPushes<us>(board.pieces(us, PAWN)) & ~board.allPieces();
    pushes |= attacks::pawnPushes<us>(pushes & Bitboard::nthRank<us, RANK_3>()) & ~board.allPieces();

    Bitboard pushThreats = attacks::pawnAttacks<us>(pushes & safe) & nonPawnEnemies;
    eval += PUSH_THREAT * pushThreats.popcount();
    return eval;
}

constexpr int safetyAdjustment(int value)
{
    return (value + std::max(value, 0) * value / 128) / 8;
}

template<Color us>
PackedScore evaluateKings(const Board& board, const EvalData& evalData, const EvalState& evalState)
{
    constexpr Color them = ~us;

    Square theirKing = board.kingSq(them);

    PackedScore eval{0, 0};

    eval += evalState.pawnShieldStormScore(us);

    Bitboard rookCheckSquares = attacks::rookAttacks(theirKing, board.allPieces());
    Bitboard bishopCheckSquares = attacks::bishopAttacks(theirKing, board.allPieces());

    Bitboard knightChecks = evalData.attackedBy[us][KNIGHT] & attacks::knightAttacks(theirKing);
    Bitboard bishopChecks = evalData.attackedBy[us][BISHOP] & bishopCheckSquares;
    Bitboard rookChecks = evalData.attackedBy[us][ROOK] & rookCheckSquares;
    Bitboard queenChecks = evalData.attackedBy[us][QUEEN] & (bishopCheckSquares | rookCheckSquares);

    Bitboard weak = ~evalData.attacked[them] | (~evalData.attackedBy2[them] & evalData.attackedBy[them][KING]);
    Bitboard safe = ~board.allPieces() & ~evalData.attacked[them] | (weak & evalData.attackedBy2[us]);

    eval += SAFE_KNIGHT_CHECK * (knightChecks & safe).popcount();
    eval += SAFE_BISHOP_CHECK * (bishopChecks & safe).popcount();
    eval += SAFE_ROOK_CHECK * (rookChecks & safe).popcount();
    eval += SAFE_QUEEN_CHECK * (queenChecks & safe).popcount();

    eval += UNSAFE_KNIGHT_CHECK * (knightChecks & ~safe).popcount();
    eval += UNSAFE_BISHOP_CHECK * (bishopChecks & ~safe).popcount();
    eval += UNSAFE_ROOK_CHECK * (rookChecks & ~safe).popcount();
    eval += UNSAFE_QUEEN_CHECK * (queenChecks & ~safe).popcount();

    bool queenless = board.pieces(us, PieceType::QUEEN).empty();
    eval += QUEENLESS_ATTACK * queenless;

    eval += evalData.attackWeight[us];

    int attackCount = evalData.attackCount[us];
    eval += KING_ATTACKS * attackCount;

    Bitboard weakKingRing = (evalData.kingRing[them] & weak);
    Bitboard weakAttacked = weakKingRing & evalData.attacked[us];
    Bitboard weakAttacked2 = weakAttacked & evalData.attackedBy2[us];
    int weakSquares = weakKingRing.popcount() + weakAttacked.popcount() + weakAttacked2.popcount();
    eval += WEAK_KING_RING * weakSquares;

    eval += SAFETY_OFFSET;

    PackedScore safety{safetyAdjustment(eval.mg()), safetyAdjustment(eval.eg())};
    return safety;
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
            bool controlled = evalData.attacked[them].has(pushSq);
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

int evaluateScale(const Board& board, PackedScore eval, const EvalState& evalState)
{
    int scaleFactor = SCALE_FACTOR_NORMAL;
    Color strongSide = eval.eg() > 0 ? WHITE : BLACK;

    auto endgameScale = endgames::probeScaleFunc(board, strongSide);
    if (endgameScale != nullptr)
        scaleFactor = (*endgameScale)(board, evalState);

    if (scaleFactor != SCALE_FACTOR_NORMAL)
        return scaleFactor;

    int strongPawns = board.pieces(strongSide, PAWN).popcount();
    return 80 + strongPawns * 7;
}

template<Color us>
void initEvalData(const Board& board, EvalData& evalData, const PawnStructure& pawnStructure)
{
    constexpr Color them = ~us;
    Bitboard ourPawns = board.pieces(us, PAWN);
    Bitboard blockedPawns = ourPawns & attacks::pawnPushes<them>(board.allPieces());
    Square ourKing = board.kingSq(us);

    evalData.mobilityArea[us] = ~pawnStructure.pawnAttacks[them] & ~Bitboard::fromSquare(ourKing) & ~blockedPawns;
    evalData.attacked[us] = evalData.attackedBy[us][PAWN] = pawnStructure.pawnAttacks[us];

    Bitboard ourKingAtks = attacks::kingAttacks(ourKing);
    evalData.attackedBy[us][KING] = ourKingAtks;
    evalData.attackedBy2[us] = evalData.attacked[us] & ourKingAtks;
    evalData.attacked[us] |= ourKingAtks;
    evalData.kingRing[us] = (ourKingAtks | attacks::pawnPushes<us>(ourKingAtks)) & ~Bitboard::fromSquare(ourKing);
    if (FILE_H_BB.has(ourKing))
        evalData.kingRing[us] |= evalData.kingRing[us].west();
    if (FILE_A_BB.has(ourKing))
        evalData.kingRing[us] |= evalData.kingRing[us].east();
}

void nonIncrementalEval(const Board& board, const EvalState& evalState, const PawnStructure& pawnStructure, EvalData& evalData, PackedScore& eval)
{
    eval += evaluatePieces<WHITE, KNIGHT>(board, evalData) - evaluatePieces<BLACK, KNIGHT>(board, evalData);
    eval += evaluatePieces<WHITE, BISHOP>(board, evalData) - evaluatePieces<BLACK, BISHOP>(board, evalData);
    eval += evaluatePieces<WHITE, ROOK>(board, evalData) - evaluatePieces<BLACK, ROOK>(board, evalData);
    eval += evaluatePieces<WHITE, QUEEN>(board, evalData) - evaluatePieces<BLACK, QUEEN>(board, evalData);

    eval += evaluateKings<WHITE>(board, evalData, evalState) - evaluateKings<BLACK>(board, evalData, evalState);
    eval += evaluatePassedPawns<WHITE>(board, pawnStructure, evalData) - evaluatePassedPawns<BLACK>(board, pawnStructure, evalData);
    eval += evaluateThreats<WHITE>(board, evalData) - evaluateThreats<BLACK>(board, evalData);
    eval += evaluateComplexity(board, pawnStructure, eval);
}

int evaluate(const Board& board, search::SearchThread* thread)
{
    auto endgameEval = endgames::probeEvalFunc(board);
    if (endgameEval != nullptr)
        return (*endgameEval)(board, thread->evalState);

    Color color = board.sideToMove();
    PackedScore eval = thread->evalState.score(board);

    const PawnStructure& pawnStructure = thread->evalState.pawnStructure();

    EvalData evalData = {};
    initEvalData<WHITE>(board, evalData, pawnStructure);
    initEvalData<BLACK>(board, evalData, pawnStructure);

    nonIncrementalEval(board, thread->evalState, pawnStructure, evalData, eval);

    int scale = evaluateScale(board, eval, thread->evalState);

    eval += (color == WHITE ? TEMPO : -TEMPO);

    return (color == WHITE ? 1 : -1) * eval::getFullEval(eval.mg(), eval.eg() * scale / SCALE_FACTOR_NORMAL, thread->evalState.phase());
}

int evaluateSingle(const Board& board)
{
    EvalState evalState;
    evalState.initSingle(board);

    auto endgame = endgames::probeEvalFunc(board);
    if (endgame != nullptr)
        return (*endgame)(board, evalState);

    Color color = board.sideToMove();
    PackedScore eval = evalState.score(board);

    const PawnStructure& pawnStructure = evalState.pawnStructure();

    EvalData evalData = {};
    initEvalData<WHITE>(board, evalData, pawnStructure);
    initEvalData<BLACK>(board, evalData, pawnStructure);

    nonIncrementalEval(board, evalState, pawnStructure, evalData, eval);

    int scale = evaluateScale(board, eval, evalState);

    eval += (color == WHITE ? TEMPO : -TEMPO);

    return (color == WHITE ? 1 : -1) * eval::getFullEval(eval.mg(), eval.eg() * scale / SCALE_FACTOR_NORMAL, evalState.phase());
}


}

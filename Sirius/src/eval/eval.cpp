#include "eval.h"
#include "../attacks.h"
#include "../util/enum_array.h"
#include "endgame.h"
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
    ColorArray<ScorePair> attackWeight;
    ColorArray<i32> attackCount;
    ColorArray<Bitboard> kingFlank;

    void addAttacks(Color color, PieceType pieceType, Bitboard attacks)
    {
        attackedBy2[color] |= attacked[color] & attacks;
        attacked[color] |= attacks;
        attackedBy[color][pieceType] |= attacks;
    }
};

using enum PieceType;
using enum Color;

template<Color us, PieceType piece>
ScorePair evaluatePieces(const Board& board, EvalData& evalData)
{
    constexpr Color them = ~us;
    constexpr Bitboard CENTER_SQUARES = (RANK_4_BB | RANK_5_BB) & (FILE_D_BB | FILE_E_BB);

    ScorePair eval = ScorePair(0, 0);
    Bitboard pieces = board.pieces(us, piece);
    if (piece == BISHOP && pieces.multiple())
        eval += BISHOP_PAIR;

    Bitboard occupancy = board.allPieces();
    if (piece == BISHOP)
        occupancy ^= board.pieces(us, BISHOP) | board.pieces(us, QUEEN);
    else if (piece == ROOK)
        occupancy ^= board.pieces(us, ROOK) | board.pieces(us, QUEEN);
    else if (piece == QUEEN)
        occupancy ^= board.pieces(us, BISHOP) | board.pieces(us, ROOK);

    while (pieces.any())
    {
        Square sq = pieces.poplsb();
        Bitboard attacks = attacks::pieceAttacks<piece>(sq, occupancy);
        if (board.checkBlockers(us).has(sq))
            attacks &= attacks::inBetweenSquares(sq, board.kingSq(us));

        evalData.addAttacks(us, piece, attacks);

        eval += MOBILITY[static_cast<i32>(piece) - static_cast<i32>(KNIGHT)]
                        [(attacks & evalData.mobilityArea[us]).popcount()];

        if (Bitboard kingRingAtks = evalData.kingRing[them] & attacks; kingRingAtks.any())
        {
            evalData.attackWeight[us] +=
                KING_ATTACKER_WEIGHT[static_cast<i32>(piece) - static_cast<i32>(KNIGHT)];
            evalData.attackCount[us] += kingRingAtks.popcount();
        }

        if (piece == BISHOP && (attacks & CENTER_SQUARES).multiple())
            eval += LONG_DIAG_BISHOP;
    }

    return eval;
}

template<Color us>
ScorePair evaluateThreats(const Board& board, const EvalData& evalData)
{
    constexpr Color them = ~us;

    ScorePair eval = ScorePair(0, 0);

    Bitboard defendedBB = evalData.attackedBy2[them] | evalData.attackedBy[them][PAWN]
        | (evalData.attacked[them] & ~evalData.attackedBy2[us]);

    Bitboard pawnThreats = evalData.attackedBy[us][PAWN] & board.pieces(them);
    while (pawnThreats.any())
    {
        PieceType threatened = getPieceType(board.pieceAt(pawnThreats.poplsb()));
        eval += THREAT_BY_PAWN[static_cast<i32>(threatened)];
    }

    Bitboard knightThreats = evalData.attackedBy[us][KNIGHT] & board.pieces(them);
    while (knightThreats.any())
    {
        Square threat = knightThreats.poplsb();
        PieceType threatened = getPieceType(board.pieceAt(threat));
        bool defended = defendedBB.has(threat);
        eval += THREAT_BY_KNIGHT[defended][static_cast<i32>(threatened)];
    }

    Bitboard bishopThreats = evalData.attackedBy[us][BISHOP] & board.pieces(them);
    while (bishopThreats.any())
    {
        Square threat = bishopThreats.poplsb();
        PieceType threatened = getPieceType(board.pieceAt(threat));
        bool defended = defendedBB.has(threat);
        eval += THREAT_BY_BISHOP[defended][static_cast<i32>(threatened)];
    }

    Bitboard rookThreats = evalData.attackedBy[us][ROOK] & board.pieces(them);
    while (rookThreats.any())
    {
        Square threat = rookThreats.poplsb();
        PieceType threatened = getPieceType(board.pieceAt(threat));
        bool defended = defendedBB.has(threat);
        eval += THREAT_BY_ROOK[defended][static_cast<i32>(threatened)];
    }

    Bitboard queenThreats = evalData.attackedBy[us][QUEEN] & board.pieces(them) & ~board.pieces(KING);
    while (queenThreats.any())
    {
        Square threat = queenThreats.poplsb();
        PieceType threatened = getPieceType(board.pieceAt(threat));
        bool defended = defendedBB.has(threat);
        eval += THREAT_BY_QUEEN[defended][static_cast<i32>(threatened)];
    }

    Bitboard kingThreats = evalData.attackedBy[us][KING] & board.pieces(them) & ~defendedBB;
    while (kingThreats.any())
    {
        PieceType threatened = getPieceType(board.pieceAt(kingThreats.poplsb()));
        eval += THREAT_BY_KING[static_cast<i32>(threatened)];
    }

    Bitboard nonPawnEnemies = board.pieces(them) & ~board.pieces(PAWN);

    Bitboard safe = ~defendedBB
        | (evalData.attacked[us] & ~evalData.attackedBy[them][PAWN] & ~evalData.attackedBy2[them]);
    Bitboard pushes = attacks::pawnPushes<us>(board.pieces(us, PAWN)) & ~board.allPieces();
    pushes |= attacks::pawnPushes<us>(pushes & Bitboard::nthRank<us, RANK_3>()) & ~board.allPieces();

    Bitboard pushThreats = attacks::pawnAttacks<us>(pushes & safe) & nonPawnEnemies;
    eval += PUSH_THREAT * pushThreats.popcount();

    Bitboard restriction =
        evalData.attackedBy2[us] & ~evalData.attackedBy2[them] & evalData.attacked[them];
    eval += RESTRICTED_SQUARES * restriction.popcount();

    Bitboard oppQueens = board.pieces(them, PieceType::QUEEN);
    if (oppQueens.one())
    {
        Square oppQueen = oppQueens.lsb();
        Bitboard knightHits = attacks::knightAttacks(oppQueen);
        Bitboard bishopHits = attacks::bishopAttacks(oppQueen, board.allPieces());
        Bitboard rookHits = attacks::rookAttacks(oppQueen, board.allPieces());

        Bitboard targets = safe & ~board.pieces(us, PieceType::PAWN);

        eval += KNIGHT_HIT_QUEEN
            * (targets & knightHits & evalData.attackedBy[us][PieceType::KNIGHT]).popcount();

        targets &= evalData.attackedBy2[us];
        eval += BISHOP_HIT_QUEEN
            * (targets & bishopHits & evalData.attackedBy[us][PieceType::BISHOP]).popcount();
        eval += ROOK_HIT_QUEEN
            * (targets & rookHits & evalData.attackedBy[us][PieceType::ROOK]).popcount();
    }

    return eval;
}

constexpr i32 safetyAdjustment(i32 value)
{
    return (value + std::max(value, 0) * value / 128) / 8;
}

template<Color us>
ScorePair evaluateKings(const Board& board, const EvalData& evalData, const EvalState& evalState)
{
    constexpr Color them = ~us;

    Square theirKing = board.kingSq(them);

    ScorePair eval = ScorePair(0, 0);

    eval += evalState.pawnShieldStormScore(us);

    Bitboard rookCheckSquares = attacks::rookAttacks(theirKing, board.allPieces());
    Bitboard bishopCheckSquares = attacks::bishopAttacks(theirKing, board.allPieces());

    Bitboard knightChecks = evalData.attackedBy[us][KNIGHT] & attacks::knightAttacks(theirKing);
    Bitboard bishopChecks = evalData.attackedBy[us][BISHOP] & bishopCheckSquares;
    Bitboard rookChecks = evalData.attackedBy[us][ROOK] & rookCheckSquares;
    Bitboard queenChecks = evalData.attackedBy[us][QUEEN] & (bishopCheckSquares | rookCheckSquares);

    Bitboard weak =
        ~evalData.attacked[them] | (~evalData.attackedBy2[them] & evalData.attackedBy[them][KING]);
    Bitboard safe = ~board.pieces(us) & (~evalData.attacked[them] | (weak & evalData.attackedBy2[us]));

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

    i32 attackCount = evalData.attackCount[us];
    eval += KING_ATTACKS * attackCount;

    Bitboard weakKingRing = (evalData.kingRing[them] & weak);
    i32 weakSquares = weakKingRing.popcount();
    eval += WEAK_KING_RING * weakSquares;

    Bitboard flankAttacks = evalData.kingFlank[them] & evalData.attacked[us];
    Bitboard flankAttacks2 = evalData.kingFlank[them] & evalData.attackedBy2[us];
    Bitboard flankDefenses = evalData.kingFlank[them] & evalData.attacked[them];
    Bitboard flankDefenses2 = evalData.kingFlank[them] & evalData.attackedBy2[them];

    eval += flankAttacks.popcount() * KING_FLANK_ATTACKS[0]
        + flankAttacks2.popcount() * KING_FLANK_ATTACKS[1];
    eval += flankDefenses.popcount() * KING_FLANK_DEFENSES[0]
        + flankDefenses2.popcount() * KING_FLANK_DEFENSES[1];

    Bitboard checkBlockers = board.checkBlockers(them);
    while (checkBlockers.any())
    {
        Square blocker = checkBlockers.poplsb();
        // this could technically pick the wrong pinner if there are 2 pinners
        // that are both aligned, but it's so rare that I doubt it matters
        Bitboard ray = attacks::alignedSquares(blocker, board.kingSq(them));
        Piece piece = board.pieceAt(blocker);
        Color color = getPieceColor(piece);
        PieceType pieceType = getPieceType(piece);
        // pinned
        if (color == them)
        {
            Square pinner = (board.pinners(them) & ray).lsb();
            PieceType pinnerPiece = getPieceType(board.pieceAt(pinner));

            eval += SAFETY_PINNED[static_cast<int>(pieceType)]
                                 [static_cast<int>(pinnerPiece) - static_cast<int>(BISHOP)];
        }
        // discovered
        else
        {
            Square discoverer = (board.discoverers(them) & ray).lsb();
            PieceType discovererPiece = getPieceType(board.pieceAt(discoverer));

            eval += SAFETY_DISCOVERED[static_cast<int>(pieceType)][static_cast<int>(discovererPiece)];
        }
    }

    eval += SAFETY_OFFSET;

    ScorePair safety{safetyAdjustment(eval.mg()), safetyAdjustment(eval.eg())};
    return safety;
}

template<Color us>
ScorePair evaluatePassedPawns(
    const Board& board, const PawnStructure& pawnStructure, const EvalData& evalData)
{
    constexpr Color them = ~us;
    Square ourKing = board.kingSq(us);
    Square theirKing = board.kingSq(them);

    Bitboard passers = pawnStructure.passedPawns & board.pieces(us);

    ScorePair eval = ScorePair(0, 0);

    while (passers.any())
    {
        Square passer = passers.poplsb();
        i32 rank = passer.relativeRank<us>();
        if (rank >= RANK_4)
        {
            Square pushSq = passer + attacks::pawnPushOffset<us>();

            bool blocked = board.pieceAt(pushSq) != Piece::NONE;
            bool controlled = evalData.attacked[them].has(pushSq);
            eval += PASSED_PAWN[blocked][controlled][rank];

            eval += OUR_PASSER_PROXIMITY[Square::chebyshev(ourKing, pushSq)];
            eval += THEIR_PASSER_PROXIMITY[Square::chebyshev(theirKing, pushSq)];

            if (evalData.attacked[us].has(pushSq))
                eval += PASSER_DEFENDED_PUSH[rank];
        }
    }

    return eval;
}

ScorePair evaluateComplexity(const Board& board, const PawnStructure& pawnStructure, ScorePair eval)
{
    constexpr Bitboard KING_SIDE = FILE_A_BB | FILE_B_BB | FILE_C_BB | FILE_D_BB;
    constexpr Bitboard QUEEN_SIDE = ~KING_SIDE;
    Bitboard pawns = board.pieces(PAWN);
    bool pawnsBothSides = (pawns & KING_SIDE).any() && (pawns & QUEEN_SIDE).any();
    bool pawnEndgame = board.allPieces() == (pawns | board.pieces(KING));

    ScorePair complexity = COMPLEXITY_PAWNS * pawns.popcount()
        + COMPLEXITY_PAWNS_BOTH_SIDES * pawnsBothSides + COMPLEXITY_PAWN_ENDGAME * pawnEndgame
        + COMPLEXITY_OFFSET;

    i32 egSign = (eval.eg() > 0) - (eval.eg() < 0);

    i32 egComplexity = std::max(complexity.eg(), -std::abs(eval.eg()));

    return ScorePair(0, egSign * egComplexity);
}

i32 evaluateScale(const Board& board, ScorePair eval, const EvalState& evalState)
{
    i32 scaleFactor = SCALE_FACTOR_NORMAL;
    Color strongSide = eval.eg() > 0 ? WHITE : eval.eg() < 0 ? BLACK : board.sideToMove();

    auto endgameScale = endgames::probeScaleFunc(board, strongSide);
    if (endgameScale != nullptr)
        scaleFactor = (*endgameScale)(board, evalState);

    if (scaleFactor != SCALE_FACTOR_NORMAL)
        return scaleFactor;

    i32 strongPawns = board.pieces(strongSide, PAWN).popcount();
    return 80 + strongPawns * 7;
}

template<Color us>
void initEvalData(const Board& board, EvalData& evalData, const PawnStructure& pawnStructure)
{
    constexpr Color them = ~us;
    Bitboard ourPawns = board.pieces(us, PAWN);
    Bitboard blockedPawns = ourPawns & attacks::pawnPushes<them>(board.allPieces());
    Square ourKing = board.kingSq(us);

    evalData.mobilityArea[us] =
        ~pawnStructure.pawnAttacks[them] & ~Bitboard::fromSquare(ourKing) & ~blockedPawns;
    evalData.addAttacks(us, PieceType::PAWN, pawnStructure.pawnAttacks[us]);

    Bitboard ourKingAtks = attacks::kingAttacks(ourKing);
    evalData.addAttacks(us, PieceType::KING, ourKingAtks);
    evalData.kingRing[us] = attacks::kingRing<us>(ourKing);
    evalData.kingFlank[us] = attacks::kingFlank(us, ourKing.file());
}

// clang-format off
void nonIncrementalEval(const Board& board, const EvalState& evalState,
    const PawnStructure& pawnStructure, EvalData& evalData, ScorePair& eval)
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
// clang-format on

i32 evaluate(const Board& board, search::SearchThread* thread)
{
    auto endgameEval = endgames::probeEvalFunc(board);
    if (endgameEval != nullptr)
        return (*endgameEval)(board, thread->evalState);

    Color color = board.sideToMove();
    ScorePair eval = thread->evalState.score(board);

    const PawnStructure& pawnStructure = thread->evalState.pawnStructure();

    EvalData evalData = {};
    initEvalData<WHITE>(board, evalData, pawnStructure);
    initEvalData<BLACK>(board, evalData, pawnStructure);

    nonIncrementalEval(board, thread->evalState, pawnStructure, evalData, eval);

    i32 scale = evaluateScale(board, eval, thread->evalState);

    eval += (color == WHITE ? TEMPO : -TEMPO);

    i32 mg = eval.mg();
    i32 eg = eval.eg() * scale / SCALE_FACTOR_NORMAL;
    i32 phase = 4 * board.pieces(PieceType::QUEEN).popcount()
        + 2 * board.pieces(PieceType::ROOK).popcount()
        + (board.pieces(PieceType::BISHOP) | board.pieces(PieceType::KNIGHT)).popcount();
    phase = std::clamp(phase, 0, 24);

    return (color == WHITE ? 1 : -1) * ((mg * phase + eg * (24 - phase)) / 24);
}

i32 evaluateSingle(const Board& board)
{
    EvalState evalState;
    evalState.initSingle(board);

    auto endgame = endgames::probeEvalFunc(board);
    if (endgame != nullptr)
        return (*endgame)(board, evalState);

    Color color = board.sideToMove();
    ScorePair eval = evalState.score(board);

    const PawnStructure& pawnStructure = evalState.pawnStructure();

    EvalData evalData = {};
    initEvalData<WHITE>(board, evalData, pawnStructure);
    initEvalData<BLACK>(board, evalData, pawnStructure);

    nonIncrementalEval(board, evalState, pawnStructure, evalData, eval);

    i32 scale = evaluateScale(board, eval, evalState);

    eval += (color == WHITE ? TEMPO : -TEMPO);

    i32 mg = eval.mg();
    i32 eg = eval.eg() * scale / SCALE_FACTOR_NORMAL;
    i32 phase = 4 * board.pieces(PieceType::QUEEN).popcount()
        + 2 * board.pieces(PieceType::ROOK).popcount()
        + (board.pieces(PieceType::BISHOP) | board.pieces(PieceType::KNIGHT)).popcount();
    phase = std::clamp(phase, 0, 24);

    return (color == WHITE ? 1 : -1) * ((mg * phase + eg * (24 - phase)) / 24);
}

}

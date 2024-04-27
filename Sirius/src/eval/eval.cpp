#include "eval.h"
#include "../attacks.h"

namespace eval
{

struct EvalData
{
    Bitboard mobilityArea;
    Bitboard attacked;
    std::array<Bitboard, 6> attackedBy;
};

template<Color color, PieceType piece>
PackedScore evaluatePieces(const Board& board, EvalData& ourData)
{
    Bitboard ourPawns = board.getPieces(color, PieceType::PAWN);
    Bitboard theirPawns = board.getPieces(~color, PieceType::PAWN);

    PackedScore eval{0, 0};
    Bitboard pieces = board.getPieces(color, piece);
    if constexpr (piece == PieceType::BISHOP)
        if (pieces.multiple())
            eval += BISHOP_PAIR;

    while (pieces)
    {
        uint32_t sq = pieces.poplsb();
        Bitboard attacks = attacks::pieceAttacks<piece>(sq, board.getAllPieces());
        ourData.attacked |= attacks;
        eval += MOBILITY[static_cast<int>(piece) - static_cast<int>(PieceType::KNIGHT)][(attacks & ourData.mobilityArea).popcount()];

        Bitboard threats = attacks & board.getColor(~color);
        while (threats)
            eval += THREATS[static_cast<int>(piece)][static_cast<int>(getPieceType(board.getPieceAt(threats.poplsb())))];

        Bitboard fileBB = Bitboard::fileBB(fileOf(sq));

        if constexpr (piece == PieceType::ROOK)
        {
            if ((ourPawns & fileBB).empty())
                eval += (theirPawns & fileBB).any() ? ROOK_OPEN[1] : ROOK_OPEN[0];
        }
    }

    return eval;
}



template<Color color>
PackedScore evaluatePawns(const Board& board)
{
    Bitboard ourPawns = board.getPieces(color, PieceType::PAWN);

    PackedScore eval{0, 0};

    Bitboard pawns = ourPawns;
    while (pawns)
    {
        uint32_t sq = pawns.poplsb();
        if (board.isPassedPawn(sq))
            eval += PASSED_PAWN[relativeRankOf<color>(sq)];
        if (board.isIsolatedPawn(sq))
            eval += ISOLATED_PAWN[fileOf(sq)];
    }
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
template<Color color>
PackedScore evaluateThreats(const Board& board, const EvalData& ourData)
{
    Bitboard threats = ourData.attackedBy[static_cast<int>(PieceType::PAWN)] & board.getColor(~color);
    PackedScore eval{0, 0};
    while (threats)
        eval += THREATS[static_cast<int>(PieceType::PAWN)][static_cast<int>(getPieceType(board.getPieceAt(threats.poplsb())))];
    return eval;
}

template<Color color>
PackedScore evaluateKings(const Board& board)
{
    Bitboard ourPawns = board.getPieces(color, PieceType::PAWN);

    uint32_t ourKing = board.getPieces(color, PieceType::KING).lsb();
    uint32_t theirKing = board.getPieces(~color, PieceType::KING).lsb();

    PackedScore eval{0, 0};

    for (uint32_t file = 0; file < 8; file++)
    {
        Bitboard filePawns = ourPawns & Bitboard::fileBB(file);
        {
            uint32_t kingFile = fileOf(theirKing);
            // 4 = e file
            int idx = (kingFile == file) ? 1 : (kingFile >= 4) == (kingFile < file) ? 0 : 2;

            int rankDist = filePawns ?
                std::abs(rankOf(color == Color::WHITE ? filePawns.msb() : filePawns.lsb()) - rankOf(theirKing)) :
                7;
            eval += PAWN_STORM[idx][rankDist];
        }
        {
            uint32_t kingFile = fileOf(ourKing);
            // 4 = e file
            int idx = (kingFile == file) ? 1 : (kingFile >= 4) == (kingFile < file) ? 0 : 2;
            int rankDist = filePawns ?
                std::abs(rankOf(color == Color::WHITE ? filePawns.lsb() : filePawns.msb()) - rankOf(ourKing)) :
                7;
            eval += PAWN_SHIELD[idx][rankDist];
        }
    }

    return eval;
}

void initEvalData(const Board& board, EvalData& white, EvalData& black)
{
    Bitboard whitePawns = board.getPieces(Color::WHITE, PieceType::PAWN);
    Bitboard blackPawns = board.getPieces(Color::BLACK, PieceType::PAWN);
    Bitboard whitePawnAttacks = attacks::pawnAttacks<Color::WHITE>(whitePawns);
    Bitboard blackPawnAttacks = attacks::pawnAttacks<Color::BLACK>(blackPawns);

    white.mobilityArea = ~blackPawnAttacks;
    white.attacked = white.attackedBy[static_cast<int>(PieceType::PAWN)] = whitePawnAttacks;
    black.mobilityArea = ~whitePawnAttacks;
    black.attacked = black.attackedBy[static_cast<int>(PieceType::PAWN)] = blackPawnAttacks;
}

int evaluate(const Board& board, search::SearchThread* thread)
{
    EvalData whiteData = {}, blackData = {};
    initEvalData(board, whiteData, blackData);

    if (!eval::canForceMate(board))
        return SCORE_DRAW;

    Color color = board.sideToMove();
    PackedScore eval = board.evalState().materialPsqt;

    eval += evaluatePieces<Color::WHITE, PieceType::KNIGHT>(board, whiteData) - evaluatePieces<Color::BLACK, PieceType::KNIGHT>(board, blackData);
    eval += evaluatePieces<Color::WHITE, PieceType::BISHOP>(board, whiteData) - evaluatePieces<Color::BLACK, PieceType::BISHOP>(board, blackData);
    eval += evaluatePieces<Color::WHITE, PieceType::ROOK>(board, whiteData) - evaluatePieces<Color::BLACK, PieceType::ROOK>(board, blackData);
    eval += evaluatePieces<Color::WHITE, PieceType::QUEEN>(board, whiteData) - evaluatePieces<Color::BLACK, PieceType::QUEEN>(board, blackData);

    eval += evaluateKings<Color::WHITE>(board) - evaluateKings<Color::BLACK>(board);

    eval += evaluatePawns(board, thread ? &thread->pawnTable : nullptr);
    eval += evaluateThreats<Color::WHITE>(board, whiteData) - evaluateThreats<Color::BLACK>(board, blackData);


    return (color == Color::WHITE ? 1 : -1) * eval::getFullEval(eval.mg(), eval.eg(), board.evalState().phase);
}


}

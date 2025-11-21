#include "endgame.h"
#include "../attacks.h"
#include "eval_constants.h"

#include <algorithm>

namespace eval::endgames
{

i32 distToAnyCorner(Square sq)
{
    i32 rankDist = std::min(sq.rank(), 7 - sq.rank());
    i32 fileDist = std::min(sq.file(), 7 - sq.file());

    return rankDist + fileDist;
}

i32 trivialDraw(const Board&, const EvalState&, Color)
{
    return 0;
}

i32 evalKXvK(const Board& board, const EvalState& evalState, Color strongSide)
{
    Color weakSide = ~strongSide;

    assert(!board.pieces(weakSide).multiple());

    Square ourKing = board.kingSq(strongSide);
    Square theirKing = board.kingSq(weakSide);

    i32 cornerDist = distToAnyCorner(theirKing);
    i32 kingDist = Square::manhattan(ourKing, theirKing);

    i32 result = evalState.psqtScore(board, strongSide).eg() + 13 * 20 - 20 * kingDist - 20 * cornerDist;

    if (board.pieces(PieceType::QUEEN).any() || board.pieces(PieceType::ROOK).any()
        || (board.pieces(PieceType::BISHOP).any() && board.pieces(PieceType::KNIGHT).any())
        || ((board.pieces(PieceType::BISHOP) & LIGHT_SQUARES_BB).any()
            && (board.pieces(PieceType::BISHOP) & DARK_SQUARES_BB).any()))
        result += 10000;

    return result;
}

i32 evalKBNvK(const Board& board, const EvalState&, Color strongSide)
{
    Color weakSide = ~strongSide;
    Square bishop = board.pieces(strongSide, PieceType::BISHOP).lsb();
    Square ourKing = board.kingSq(strongSide);
    Square theirKing = board.kingSq(weakSide);

    i32 correctCornerDist = std::abs(7 - theirKing.rank() - theirKing.file());
    if (bishop.darkSquare())
        correctCornerDist = 7 - correctCornerDist;

    i32 cornerDist = distToAnyCorner(theirKing);

    i32 kingDist = Square::manhattan(ourKing, theirKing);

    return 10000 - kingDist * 20 - cornerDist * 20 - correctCornerDist * 200;
}

i32 evalKQvKP(const Board& board, const EvalState&, Color strongSide)
{
    Color weakSide = ~strongSide;
    Square queen = board.pieces(strongSide, PieceType::QUEEN).lsb();
    Square pawn = board.pieces(weakSide, PieceType::PAWN).lsb();
    Square ourKing = board.kingSq(strongSide);

    i32 kpDist = Square::chebyshev(ourKing, pawn);

    i32 eval = 140 - 20 * kpDist;

    Bitboard queeningSquares = attacks::fillUp(Bitboard::fromSquare(pawn), weakSide);
    // if queen or king is blocking pawn it is guaranteed win
    // the king can't be attacking the queen since then it would be in check, and eval is not called in check
    if (queeningSquares.has(queen) || queeningSquares.has(ourKing))
        eval += 10000;

    if (pawn.relativeRank(weakSide) < RANK_7
        || (FILE_B_BB | FILE_D_BB | FILE_E_BB | FILE_G_BB).has(pawn) || eval >= 10000)
    {
        eval += MATERIAL[static_cast<i32>(PieceType::QUEEN)].eg()
            - MATERIAL[static_cast<i32>(PieceType::PAWN)].eg();
    }

    return eval;
}

i32 evalKQvKR(const Board& board, const EvalState& evalState, Color strongSide)
{
    Color weakSide = ~strongSide;

    Square ourKing = board.kingSq(strongSide);
    Square theirKing = board.kingSq(weakSide);

    i32 cornerDist = distToAnyCorner(theirKing);
    i32 kingDist = Square::manhattan(ourKing, theirKing);

    return evalState.psqtScore(board, strongSide).eg() + 13 * 20 - 20 * kingDist - 20 * cornerDist;
}

i32 scaleKPsvK(const Board& board, const EvalState&, Color strongSide)
{
    Bitboard strongPawns = board.pieces(strongSide, PieceType::PAWN);
    Square weakKing = board.kingSq(~strongSide);
    i32 queeningRank = strongSide == Color::WHITE ? RANK_8 : RANK_1;

    if ((strongPawns & ~FILE_A_BB).empty() || (strongPawns & ~FILE_H_BB).empty())
    {
        Square queeningSquare = Square(queeningRank, strongPawns.lsb().file());
        if (Square::chebyshev(weakKing, queeningSquare) <= 1)
            return SCALE_FACTOR_DRAW;
    }
    return SCALE_FACTOR_NORMAL;
}

i32 scaleKBPsvK(const Board& board, const EvalState&, Color strongSide)
{
    Bitboard strongPawns = board.pieces(strongSide, PieceType::PAWN);
    Square bishop = board.pieces(strongSide, PieceType::BISHOP).lsb();
    Square weakKing = board.kingSq(~strongSide);
    i32 queeningRank = strongSide == Color::WHITE ? RANK_8 : RANK_1;

    if ((strongPawns & ~FILE_A_BB).empty() || (strongPawns & ~FILE_H_BB).empty())
    {
        Square queeningSquare = Square(queeningRank, strongPawns.lsb().file());
        if (queeningSquare.darkSquare() != bishop.darkSquare()
            && Square::chebyshev(weakKing, queeningSquare) <= 1)
            return SCALE_FACTOR_DRAW;
    }
    return SCALE_FACTOR_NORMAL;
}

using Key = u64;

Key genMaterialKey(std::string white, std::string black)
{
    assert(white.length() < 8);
    assert(black.length() < 8);

    std::transform(white.begin(), white.end(), white.begin(),
        [](auto c)
        {
            return std::toupper(c);
        });
    std::transform(black.begin(), black.end(), black.begin(),
        [](auto c)
        {
            return std::tolower(c);
        });

    std::string fen;
    fen += black;
    fen += static_cast<char>('0' + 8 - black.length());
    fen += "/8/8/8/8/8/8/";
    fen += white;
    fen += static_cast<char>('0' + 8 - white.length());
    fen += " w - - 0 1";

    Board board;
    board.setToFen(fen);

    return board.materialKey();
}

constexpr usize ENDGAME_TABLE_SIZE = 2048;
std::array<Endgame, ENDGAME_TABLE_SIZE> endgameEvalTable;

void insertEndgame(u64 key, Endgame endgame)
{
    usize idx = key % ENDGAME_TABLE_SIZE;
    if (endgameEvalTable[idx].func != nullptr)
    {
        std::cerr << "Endgame table collision" << std::endl;
        throw std::runtime_error("Endgame table collision");
    }
    endgame.key = key;
    endgameEvalTable[idx] = endgame;
}

void addEndgameEval(const std::string& strongCode, const std::string& weakCode, EndgameFunc* func)
{
    insertEndgame(genMaterialKey(strongCode, weakCode), Endgame(Color::WHITE, func, EndgameType::EVAL));
    if (strongCode != weakCode)
        insertEndgame(
            genMaterialKey(weakCode, strongCode), Endgame(Color::BLACK, func, EndgameType::EVAL));
}

bool isKXvK(const Board& board, Color strongSide)
{
    if (board.pieces(~strongSide).multiple())
        return false;

    Bitboard minors = board.pieces(PieceType::BISHOP) | board.pieces(PieceType::KNIGHT);
    return board.pieces(PieceType::QUEEN).any() || board.pieces(PieceType::ROOK).any()
        || minors.multiple();
}

bool isKPsvK(const Board& board, Color strongSide)
{
    Bitboard pawns = board.pieces(strongSide, PieceType::PAWN);
    return (board.pieces(strongSide) ^ pawns).one() && pawns.any();
}

bool isKBPsvK(const Board& board, Color strongSide)
{
    Bitboard bishops = board.pieces(strongSide, PieceType::BISHOP);
    Bitboard pawns = board.pieces(strongSide, PieceType::PAWN);
    return (board.pieces(strongSide) ^ bishops ^ pawns).one() && bishops.one() && pawns.any();
}

// clang-format off
ColorArray<Endgame> evalKXvKEndgames = {
    Endgame(Color::WHITE, &evalKXvK, EndgameType::EVAL),
    Endgame(Color::BLACK, &evalKXvK, EndgameType::EVAL)
};

ColorArray<Endgame> scaleKBPsvKEndgames = {
    Endgame(Color::WHITE, &scaleKBPsvK, EndgameType::SCALE),
    Endgame(Color::BLACK, &scaleKBPsvK, EndgameType::SCALE)
};

ColorArray<Endgame> scaleKPsvKEndgames = {
    Endgame(Color::WHITE, &scaleKPsvK, EndgameType::SCALE),
    Endgame(Color::BLACK, &scaleKPsvK, EndgameType::SCALE)
};
// clang-format on

const Endgame* probeEvalFunc(const Board& board)
{
    if (!board.pieces(PieceType::PAWN).multiple())
    {
        u64 materialKey = board.materialKey();
        usize idx = materialKey % ENDGAME_TABLE_SIZE;
        if (endgameEvalTable[idx].func != nullptr && endgameEvalTable[idx].key == materialKey)
            return &endgameEvalTable[idx];
    }

    for (Color c : {Color::WHITE, Color::BLACK})
    {
        if (isKXvK(board, c))
            return &evalKXvKEndgames[c];
    }

    return nullptr;
}

const Endgame* probeScaleFunc(const Board& board, Color strongSide)
{
    if (isKPsvK(board, strongSide))
        return &scaleKPsvKEndgames[strongSide];

    if (isKBPsvK(board, strongSide))
        return &scaleKBPsvKEndgames[strongSide];

    return nullptr;
}

void init()
{
    addEndgameEval("K", "K", &trivialDraw);
    addEndgameEval("KN", "K", &trivialDraw);
    addEndgameEval("KB", "K", &trivialDraw);
    addEndgameEval("KN", "KN", &trivialDraw);
    addEndgameEval("KB", "KN", &trivialDraw);
    addEndgameEval("KB", "KB", &trivialDraw);
    addEndgameEval("KNN", "K", &trivialDraw);

    addEndgameEval("KBN", "K", &evalKBNvK);

    addEndgameEval("KQ", "KP", &evalKQvKP);
    addEndgameEval("KQ", "KR", &evalKQvKR);
}

}

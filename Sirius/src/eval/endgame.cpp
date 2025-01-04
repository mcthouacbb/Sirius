#include "endgame.h"
#include "eval_constants.h"

#include <algorithm>
#include <unordered_map>

namespace eval::endgames
{

int distToAnyCorner(Square sq)
{
    int rankDist = std::min(sq.rank(), 7 - sq.rank());
    int fileDist = std::min(sq.file(), 7 - sq.file());

    return rankDist + fileDist;
}

int endgameMaterial(const Board& board, Color c)
{
    int mat = 0;
    Bitboard pieces = board.pieces(c) & ~board.pieces(PieceType::KING);
    while (pieces.any())
    {
        PieceType type = getPieceType(board.pieceAt(pieces.poplsb()));
        mat += MATERIAL[static_cast<int>(type)].eg();
    }

    return mat;
}

int trivialDraw(const Board&, Color)
{
    return 0;
}

int KXvK(const Board& board, Color strongSide)
{
    Color weakSide = ~strongSide;

    assert(!board.pieces(weakSide).multiple());

    Square ourKing = board.pieces(strongSide, PieceType::KING).lsb();
    Square theirKing = board.pieces(weakSide, PieceType::KING).lsb();

    int cornerDist = distToAnyCorner(theirKing);
    int kingDist = Square::manhattan(ourKing, theirKing);

    int result = endgameMaterial(board, strongSide) + 13 * 20 - 20 * kingDist - 20 * cornerDist;
    
    if (board.pieces(PieceType::QUEEN).any() ||
        board.pieces(PieceType::ROOK).any() ||
        (board.pieces(PieceType::BISHOP).any() && board.pieces(PieceType::KNIGHT).any()) ||
        ((board.pieces(PieceType::BISHOP) & LIGHT_SQUARES_BB).any() && (board.pieces(PieceType::BISHOP) & DARK_SQUARES_BB).any()))
        result += 10000;

    return result;
}

int KBNvK(const Board& board, Color strongSide)
{
    Color weakSide = ~strongSide;
    Square bishop = board.pieces(strongSide, PieceType::BISHOP).lsb();
    Square ourKing = board.pieces(strongSide, PieceType::KING).lsb();
    Square theirKing = board.pieces(weakSide, PieceType::KING).lsb();

    int correctCornerDist = std::abs(7 - theirKing.rank() - theirKing.file());
    if (bishop.darkSquare())
        correctCornerDist = 7 - correctCornerDist;

    int cornerDist = distToAnyCorner(theirKing);

    int kingDist = Square::manhattan(ourKing, theirKing);

    return 10000 - kingDist * 20 - cornerDist * 20 - correctCornerDist * 200;
}

using Key = uint64_t;
using Map = std::unordered_map<Key, Endgame>;

Key genMaterialKey(std::string white, std::string black)
{
    assert(white.length() < 8);
    assert(black.length() < 8);

    std::transform(white.begin(), white.end(), white.begin(), [](auto c) { return std::toupper(c); });
    std::transform(black.begin(), black.end(), black.begin(), [](auto c) { return std::tolower(c); });

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

constexpr size_t ENDGAME_TABLE_SIZE = 2048;
std::array<Endgame, ENDGAME_TABLE_SIZE> endgameTable;

void insertEndgame(uint64_t key, Endgame endgame)
{
    size_t idx = key % ENDGAME_TABLE_SIZE;
    if (endgameTable[idx].func != nullptr)
    {
        std::cerr << "Endgame table collision" << std::endl;
        throw std::runtime_error("Endgame table collision");
    }
    endgame.materialKey = key;
    endgameTable[idx] = endgame;
}

void addEndgameFunc(const std::string& strongCode, const std::string& weakCode, EndgameFunc* func)
{
    insertEndgame(genMaterialKey(strongCode, weakCode), Endgame(Color::WHITE, func));
    if (strongCode != weakCode)
        insertEndgame(genMaterialKey(weakCode, strongCode), Endgame(Color::BLACK, func));
}

bool isKXvK(const Board& board, Color strongSide)
{
    if (board.pieces(~strongSide).multiple())
        return false;

    Bitboard minors = board.pieces(PieceType::BISHOP) | board.pieces(PieceType::KNIGHT);
    return
        board.pieces(PieceType::QUEEN).any() ||
        board.pieces(PieceType::ROOK).any() ||
        minors.multiple();
}

Endgame KXvKEndgames[] = {
    Endgame(Color::WHITE, &KXvK), Endgame(Color::BLACK, &KXvK)
};

const Endgame* probe(const Board& board)
{
    uint64_t materialKey = board.materialKey();
    size_t idx = materialKey % ENDGAME_TABLE_SIZE;
    if (endgameTable[idx].materialKey == materialKey)
        return &endgameTable[idx];

    for (Color c : {Color::WHITE, Color::BLACK})
    {
        if (isKXvK(board, c))
            return &KXvKEndgames[static_cast<int>(c)];
    }

    return nullptr;
}

void init()
{
    addEndgameFunc("K", "K", trivialDraw);
    addEndgameFunc("KN", "K", trivialDraw);
    addEndgameFunc("KB", "K", trivialDraw);
    addEndgameFunc("KN", "KN", trivialDraw);
    addEndgameFunc("KB", "KN", trivialDraw);
    addEndgameFunc("KNN", "K", trivialDraw);

    addEndgameFunc("KBN", "K", KBNvK);
}


}

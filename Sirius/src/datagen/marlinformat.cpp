#include "marlinformat.h"
#include "../util/static_vector.h"
#include <tuple>

namespace marlinformat
{

enum class PieceCode
{
    PAWN,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING,
    CASTLING_ROOK
};

PackedBoard packBoard(const Board& board, i32 score, WDL wdl)
{
    PackedBoard result = {};
    Bitboard occ = board.allPieces();
    result.occ = occ.value();
    usize index = 0;
    while (occ.any())
    {
        Square sq = occ.poplsb();
        Piece piece = board.pieceAt(sq);
        Color color = getPieceColor(piece);
        PieceType pieceType = getPieceType(piece);
        PieceCode code = static_cast<PieceCode>(pieceType);

        if (pieceType == PieceType::ROOK)
        {
            CastleSide side = board.kingSq(color).file() < sq.file() ? CastleSide::KING_SIDE
                                                                     : CastleSide::QUEEN_SIDE;
            if (board.castlingRights().has(CastlingRights(color, side))
                && board.castlingRookSq(color, side) == sq)
            {
                code = PieceCode::CASTLING_ROOK;
            }
        }

        result.pieces.set(index++, (static_cast<u8>(color) << 3) | static_cast<u8>(code));
    }

    i32 epSquare = board.epSquare() == -1 ? 64 : board.epSquare();

    result.stmEpSquare |= (static_cast<u8>(board.sideToMove()) << 7) | epSquare;
    result.halfMoveClock = board.halfMoveClock();
    result.fullMoveNumber = board.gamePly() / 2 + 1;
    result.score = score;
    result.wdl = wdl;
    return result;
}

MarlinFormatUnpack unpackBoard(const PackedBoard& packedBoard)
{
    BoardState state = {};
    state.squares.fill(Piece::NONE);

    Bitboard occ = Bitboard(packedBoard.occ);
    usize index = 0;
    ColorArray<StaticVector<Square, 2>> castlingRooks = {};
    ColorArray<Square> kingSquares = {};
    while (occ.any())
    {
        Square sq = occ.poplsb();
        i32 piece = packedBoard.pieces.get(index++);
        Color color = static_cast<Color>(piece >> 3);
        PieceCode code = static_cast<PieceCode>(piece & 0x7);

        PieceType pieceType =
            code == PieceCode::CASTLING_ROOK ? PieceType::ROOK : static_cast<PieceType>(code);

        state.addPiece(sq, makePiece(pieceType, color));
        if (code == PieceCode::CASTLING_ROOK)
            castlingRooks[color].push_back(sq);

        if (pieceType == PieceType::KING)
            kingSquares[color] = sq;
    }

    Color stm = static_cast<Color>(packedBoard.stmEpSquare >> 7);
    if (stm == Color::BLACK)
        state.zkey.flipSideToMove();

    state.castlingRights = CastlingRights::NONE;
    CastlingData castlingData;
    castlingData.setKingSquares(kingSquares[Color::WHITE], kingSquares[Color::BLACK]);
    for (Color color : {Color::WHITE, Color::BLACK})
    {
        for (Square sq : castlingRooks[color])
        {
            CastleSide side = sq.file() > kingSquares[color].file() ? CastleSide::KING_SIDE
                                                                    : CastleSide::QUEEN_SIDE;
            castlingData.setRookSquare(color, side, sq);
            state.castlingRights |= CastlingRights(color, side);
        }
    }

    state.zkey.updateCastlingRights(state.castlingRights);

    state.epSquare = packedBoard.stmEpSquare & 0x7F;
    if (state.epSquare == 64)
        state.epSquare = -1;
    else
        state.zkey.updateEP(state.epSquare & 7);

    state.halfMoveClock = packedBoard.halfMoveClock;

    i32 gamePly = 2 * packedBoard.fullMoveNumber - 2 + (stm == Color::BLACK);

    Board board(state, castlingData, stm, gamePly);
    return {board, packedBoard.score, packedBoard.wdl};
}

}

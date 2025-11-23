#pragma once

#include "bitboard.h"
#include "castling.h"
#include "defs.h"
#include "util/enum_array.h"
#include "util/murmur.h"
#include "zobrist.h"

#include <array>
#include <string>
#include <string_view>
#include <vector>

struct CheckInfo
{
    Bitboard checkers;
    std::array<Bitboard, 2> pinners;
    std::array<Bitboard, 2> blockers;
};

struct BoardState
{
    std::array<Piece, 64> squares;
    std::array<Bitboard, 6> pieces;
    std::array<Bitboard, 2> colors;

    i32 halfMoveClock;
    i32 pliesFromNull;
    i32 epSquare;
    CastlingRights castlingRights;
    i32 repetitions;
    i32 lastRepetition;
    ZKey zkey;
    ColorArray<ZKey> nonPawnKeys;
    ZKey minorPieceKey;
    ZKey majorPieceKey;
    ZKey pawnKey;
    CheckInfo checkInfo;
    Bitboard threats;
    Bitboard winningThreats;

    void addPiece(Square pos, Color color, PieceType pieceType)
    {
        squares[pos.value()] = makePiece(pieceType, color);

        Bitboard posBB = Bitboard::fromSquare(pos);
        pieces[static_cast<i32>(pieceType)] |= posBB;
        colors[static_cast<i32>(color)] |= posBB;

        zkey.addPiece(pieceType, color, pos);
        if (pieceType == PieceType::PAWN)
            pawnKey.addPiece(pieceType, color, pos);
        else
        {
            nonPawnKeys[color].addPiece(pieceType, color, pos);
            if (pieceType == PieceType::BISHOP || pieceType == PieceType::KNIGHT
                || pieceType == PieceType::KING)
                minorPieceKey.addPiece(pieceType, color, pos);
            if (pieceType == PieceType::ROOK || pieceType == PieceType::QUEEN
                || pieceType == PieceType::KING)
                majorPieceKey.addPiece(pieceType, color, pos);
        }
    }

    void addPiece(Square pos, Piece piece)
    {
        squares[pos.value()] = piece;
        Bitboard posBB = Bitboard::fromSquare(pos);
        PieceType pieceType = getPieceType(piece);
        Color color = getPieceColor(piece);
        pieces[static_cast<i32>(pieceType)] |= posBB;
        colors[static_cast<i32>(color)] |= posBB;

        zkey.addPiece(pieceType, color, pos);
        if (pieceType == PieceType::PAWN)
            pawnKey.addPiece(pieceType, color, pos);
        else
        {
            nonPawnKeys[color].addPiece(pieceType, color, pos);
            if (pieceType == PieceType::BISHOP || pieceType == PieceType::KNIGHT
                || pieceType == PieceType::KING)
                minorPieceKey.addPiece(pieceType, color, pos);
            if (pieceType == PieceType::ROOK || pieceType == PieceType::QUEEN
                || pieceType == PieceType::KING)
                majorPieceKey.addPiece(pieceType, color, pos);
        }
    }

    void removePiece(Square pos)
    {
        Bitboard posBB = Bitboard::fromSquare(pos);
        Piece piece = squares[pos.value()];
        PieceType pieceType = getPieceType(piece);
        Color color = getPieceColor(piece);
        squares[pos.value()] = Piece::NONE;
        pieces[static_cast<i32>(pieceType)] ^= posBB;
        colors[static_cast<i32>(color)] ^= posBB;

        zkey.removePiece(pieceType, color, pos);
        if (pieceType == PieceType::PAWN)
            pawnKey.removePiece(pieceType, color, pos);
        else
        {
            nonPawnKeys[color].removePiece(pieceType, color, pos);
            if (pieceType == PieceType::BISHOP || pieceType == PieceType::KNIGHT
                || pieceType == PieceType::KING)
                minorPieceKey.removePiece(pieceType, color, pos);
            if (pieceType == PieceType::ROOK || pieceType == PieceType::QUEEN
                || pieceType == PieceType::KING)
                majorPieceKey.removePiece(pieceType, color, pos);
        }
    }

    void movePiece(Square src, Square dst)
    {
        Piece piece = squares[src.value()];
        Bitboard srcBB = Bitboard::fromSquare(src);
        Bitboard dstBB = Bitboard::fromSquare(dst);
        Bitboard moveBB = srcBB | dstBB;
        PieceType pieceType = getPieceType(piece);
        Color color = getPieceColor(piece);

        squares[dst.value()] = piece;
        squares[src.value()] = Piece::NONE;
        pieces[static_cast<i32>(pieceType)] ^= moveBB;
        colors[static_cast<i32>(color)] ^= moveBB;

        zkey.movePiece(pieceType, color, src, dst);
        if (pieceType == PieceType::PAWN)
            pawnKey.movePiece(pieceType, color, src, dst);
        else
        {
            nonPawnKeys[color].movePiece(pieceType, color, src, dst);
            if (pieceType == PieceType::BISHOP || pieceType == PieceType::KNIGHT
                || pieceType == PieceType::KING)
                minorPieceKey.movePiece(pieceType, color, src, dst);
            if (pieceType == PieceType::ROOK || pieceType == PieceType::QUEEN
                || pieceType == PieceType::KING)
                majorPieceKey.movePiece(pieceType, color, src, dst);
        }
    }
};

namespace eval
{
struct EvalState;
struct EvalUpdates;
}

class Board
{
public:
    static constexpr const char* defaultFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    Board();
    Board(const BoardState& state, const CastlingData& castlingData, Color stm, i32 gamePly);

    void setToFen(const std::string_view& fen, bool frc = false);

    std::string stringRep() const;
    std::string fenStr() const;

    void makeMove(Move move);
    void makeMove(Move move, eval::EvalState& evalState);
    void unmakeMove();
    void unmakeMove(eval::EvalState& evalState);
    void makeNullMove();
    void unmakeNullMove();

    bool isFRC() const;
    Color sideToMove() const;
    i32 epSquare() const;
    i32 gamePly() const;
    i32 halfMoveClock() const;
    CastlingRights castlingRights() const;
    i32 pliesFromNull() const;
    ZKey zkey() const;
    ZKey pawnKey() const;
    ZKey nonPawnKey(Color color) const;
    ZKey minorPieceKey() const;
    ZKey majorPieceKey() const;
    u64 materialKey() const;

    bool isDraw(i32 searchPly) const;
    bool is3FoldDraw(i32 searchPly) const;
    bool is50MoveDraw() const;
    bool isInsufMaterialDraw() const;
    bool hasUpcomingRepetition(i32 searchPly) const;

    Piece pieceAt(Square square) const;
    Bitboard pieces(PieceType type) const;
    Bitboard pieces(Color color, PieceType type) const;
    Bitboard pieces(Color color) const;
    Bitboard allPieces() const;
    Square kingSq(Color color) const;
    Square castlingRookSq(Color color, CastleSide side) const;

    bool squareAttacked(Color color, Square square) const;
    bool squareAttacked(Color color, Square square, Bitboard blockers) const;
    Bitboard attackersTo(Color color, Square square) const;
    Bitboard attackersTo(Color color, Square square, Bitboard blockers) const;
    Bitboard attackersTo(Square square) const;
    Bitboard attackersTo(Square square, Bitboard blockers) const;
    bool castlingBlocked(Color color, CastleSide side) const;

    Bitboard pinnersBlockers(Square square, Bitboard attackers, Bitboard& pinners) const;

    Bitboard checkers() const;
    Bitboard checkBlockers(Color color) const;
    Bitboard threats() const;
    Bitboard winningThreats() const;

    bool see(Move move, i32 margin) const;
    bool isPseudoLegal(Move move) const;
    bool isLegal(Move move) const;
    ZKey keyAfter(Move move) const;

private:
    template<bool updateEval>
    void makeMove(Move move, eval::EvalState* evalState);

    template<bool updateEval>
    void unmakeMove(eval::EvalState* evalState);

    const BoardState& currState() const;
    BoardState& currState();
    Bitboard pinners(Color color) const;

    void updateCheckInfo();
    void calcThreats();
    void calcRepetitions();
    void addPiece(Square pos, Color color, PieceType pieceType, eval::EvalUpdates& updates);
    void addPiece(Square pos, Piece piece, eval::EvalUpdates& updates);
    void removePiece(Square pos, eval::EvalUpdates& updates);
    void movePiece(Square src, Square dst, eval::EvalUpdates& updates);

    i32 seePieceValue(PieceType type) const;

    static constexpr std::array<i32, 6> SEE_PIECE_VALUES = {100, 450, 450, 675, 1300, 0};

    std::vector<BoardState> m_States;
    CastlingData m_CastlingData;
    bool m_FRC;

    Color m_SideToMove;

    i32 m_GamePly;
};

inline const BoardState& Board::currState() const
{
    return m_States.back();
}

inline void Board::makeMove(Move move)
{
    makeMove<false>(move, nullptr);
}

inline void Board::makeMove(Move move, eval::EvalState& evalState)
{
    makeMove<true>(move, &evalState);
}

inline void Board::unmakeMove()
{
    unmakeMove<false>(nullptr);
}

inline void Board::unmakeMove(eval::EvalState& evalState)
{
    unmakeMove<true>(&evalState);
}

inline BoardState& Board::currState()
{
    return m_States.back();
}

inline bool Board::isDraw(i32 searchPly) const
{
    return is50MoveDraw() || isInsufMaterialDraw() || is3FoldDraw(searchPly);
}

inline bool Board::is3FoldDraw(i32 searchPly) const
{
    return currState().repetitions > 1
        || (currState().repetitions == 1 && currState().lastRepetition < searchPly);
}

inline bool Board::isFRC() const
{
    return m_FRC;
}

inline Color Board::sideToMove() const
{
    return m_SideToMove;
}

inline i32 Board::epSquare() const
{
    return currState().epSquare;
}

inline i32 Board::gamePly() const
{
    return m_GamePly;
}

inline i32 Board::halfMoveClock() const
{
    return currState().halfMoveClock;
}

inline i32 Board::pliesFromNull() const
{
    return currState().pliesFromNull;
}

inline CastlingRights Board::castlingRights() const
{
    return currState().castlingRights;
}

inline ZKey Board::zkey() const
{
    return currState().zkey;
}

inline ZKey Board::pawnKey() const
{
    return currState().pawnKey;
}

inline ZKey Board::nonPawnKey(Color color) const
{
    return currState().nonPawnKeys[color];
}

inline ZKey Board::minorPieceKey() const
{
    return currState().minorPieceKey;
}

inline ZKey Board::majorPieceKey() const
{
    return currState().majorPieceKey;
}

// yoinked from motor, which I think yoinked from Caissa
inline u64 Board::materialKey() const
{
    u64 material_key = 0;

    using enum Color;
    using enum PieceType;

    for (Color c : {WHITE, BLACK})
    {
        for (PieceType pt : {PAWN, KNIGHT, BISHOP, ROOK, QUEEN})
        {
            i32 shift = static_cast<i32>(pt) * 6 + static_cast<i32>(c) * 30;
            u64 count = pieces(c, pt).popcount();
            material_key |= (count << shift);
        }
    }

    return murmurHash3(material_key);
}

inline Piece Board::pieceAt(Square square) const
{
    return currState().squares[square.value()];
}

inline Bitboard Board::pieces(PieceType type) const
{
    return currState().pieces[static_cast<i32>(type)];
}

inline Bitboard Board::pieces(Color color, PieceType type) const
{
    return pieces(type) & pieces(color);
}

inline Bitboard Board::pieces(Color color) const
{
    return currState().colors[static_cast<i32>(color)];
}

inline Bitboard Board::allPieces() const
{
    return currState().colors[0] | currState().colors[1];
}

inline Square Board::kingSq(Color color) const
{
    return pieces(color, PieceType::KING).lsb();
}

inline Square Board::castlingRookSq(Color color, CastleSide side) const
{
    return m_CastlingData.rookSquare(color, side);
}

inline bool Board::squareAttacked(Color color, Square square) const
{
    return squareAttacked(color, square, allPieces());
}

inline Bitboard Board::attackersTo(Color color, Square square) const
{
    return attackersTo(color, square, allPieces());
}

inline Bitboard Board::attackersTo(Square square) const
{
    return attackersTo(square, allPieces());
}

inline Bitboard Board::checkers() const
{
    return currState().checkInfo.checkers;
}

inline Bitboard Board::pinners(Color color) const
{
    return currState().checkInfo.pinners[static_cast<i32>(color)];
}

inline Bitboard Board::checkBlockers(Color color) const
{
    return currState().checkInfo.blockers[static_cast<i32>(color)];
}

inline Bitboard Board::threats() const
{
    return currState().threats;
}

inline Bitboard Board::winningThreats() const
{
    return currState().winningThreats;
}

inline i32 Board::seePieceValue(PieceType type) const
{
    return SEE_PIECE_VALUES[static_cast<i32>(type)];
}

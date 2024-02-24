#pragma once

#include "defs.h"
#include "bitboard.h"
#include "eval/eval_state.h"
#include "zobrist.h"

#include <string_view>
#include <string>
#include <array>

struct CheckInfo
{
    Bitboard checkers;
    std::array<Bitboard, 2> pinners;
    std::array<Bitboard, 2> blockers;

};

struct BoardState
{
    int halfMoveClock;
    int pliesFromNull;
    int epSquare;
    int castlingRights;
    int repetitions;
    int lastRepetition;
    ZKey zkey;
    ZKey pawnKey;
    CheckInfo checkInfo;

    Piece capturedPiece;

    BoardState* prev;
};

class Board
{
public:
    static constexpr const char* defaultFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    Board();
    Board(BoardState& rootState);

    Board(const Board&) = delete;
    Board& operator=(const Board&) = delete;

    Board(Board&&) = default;
    Board& operator=(Board&&) = default;

    void setToFen(const std::string_view& fen);
    void setToEpd(const std::string_view& epd);
    void setState(const Board& board, BoardState& currState, BoardState& rootState);

    std::string stringRep() const;
    std::string fenStr() const;
    std::string epdStr() const;

    void makeMove(Move move, BoardState& state);
    void unmakeMove(Move move);
    void makeNullMove(BoardState& state);
    void unmakeNullMove();

    Color sideToMove() const;
    int epSquare() const;
    int gamePly() const;
    int halfMoveClock() const;
    int castlingRights() const;
    int pliesFromNull() const;
    ZKey zkey() const;
    ZKey pawnKey() const;

    bool isDraw(int searchPly);
    bool is3FoldDraw(int searchPly);
    bool is50MoveDraw();

    Piece getPieceAt(uint32_t square) const;
    Bitboard getPieces(PieceType type) const;
    Bitboard getPieces(Color color, PieceType type) const;
    Bitboard getColor(Color color) const;
    Bitboard getAllPieces() const;

    bool squareAttacked(Color color, uint32_t square) const;
    bool squareAttacked(Color color, uint32_t square, Bitboard blockers) const;
    Bitboard attackersTo(Color color, uint32_t square) const;
    Bitboard attackersTo(Color color, uint32_t square, Bitboard blockers) const;
    Bitboard attackersTo(uint32_t square) const;
    Bitboard attackersTo(uint32_t square, Bitboard blockers) const;

    bool isPassedPawn(uint32_t square) const;

    Bitboard pinnersBlockers(uint32_t square, Bitboard attackers, Bitboard& pinners) const;

    Bitboard checkers() const;
    Bitboard checkBlockers(Color color) const;

    bool see(Move move, int margin) const;
    bool isLegal(Move move) const;
    ZKey keyAfter(Move move) const;

    const eval::EvalState& evalState() const;
private:
    Bitboard pinners(Color color) const;

    void updateCheckInfo();
    void calcRepetitions();
    void addPiece(int pos, Color color, PieceType piece);
    void addPiece(int pos, Piece piece);
    void removePiece(int pos);
    void movePiece(int src, int dst);

    int seePieceValue(PieceType type) const;

    static constexpr std::array<int, 6> SEE_PIECE_VALUES = {
        100, 450, 450, 675, 1300, 0
    };

    std::array<Piece, 64> m_Squares;
    std::array<Bitboard, 7> m_Pieces;
    std::array<Bitboard, 2> m_Colors;

    Color m_SideToMove;

    eval::EvalState m_EvalState;

    int m_GamePly;

    BoardState* m_State;
    BoardState* m_RootState;
};

inline bool Board::isDraw(int searchPly)
{
    return is50MoveDraw() || is3FoldDraw(searchPly);
}

inline bool Board::is3FoldDraw(int searchPly)
{
    return m_State->repetitions > 1 || (m_State->repetitions == 1 && m_State->lastRepetition < searchPly);
}

inline bool Board::is50MoveDraw()
{
    return m_State->halfMoveClock >= 100;
}

inline Color Board::sideToMove() const
{
    return m_SideToMove;
}

inline int Board::epSquare() const
{
    return m_State->epSquare;
}

inline int Board::gamePly() const
{
    return m_GamePly;
}

inline int Board::halfMoveClock() const
{
    return m_State->halfMoveClock;
}

inline int Board::pliesFromNull() const
{
    return m_State->pliesFromNull;
}

inline int Board::castlingRights() const
{
    return m_State->castlingRights;
}

inline ZKey Board::zkey() const
{
    return m_State->zkey;
}

inline ZKey Board::pawnKey() const
{
    return m_State->pawnKey;
}

inline Piece Board::getPieceAt(uint32_t square) const
{
    return m_Squares[square];
}

inline Bitboard Board::getPieces(PieceType type) const
{
    return m_Pieces[static_cast<int>(type)];
}

inline Bitboard Board::getPieces(Color color, PieceType type) const
{
    return getPieces(type) & getColor(color);
}

inline Bitboard Board::getColor(Color color) const
{
    return m_Colors[static_cast<int>(color)];
}

inline Bitboard Board::getAllPieces() const
{
    return m_Pieces[0];
}

inline bool Board::squareAttacked(Color color, uint32_t square) const
{
    return squareAttacked(color, square, getAllPieces());
}

inline Bitboard Board::attackersTo(Color color, uint32_t square) const
{
    return attackersTo(color, square, getAllPieces());
}

inline Bitboard Board::attackersTo(uint32_t square) const
{
    return attackersTo(square, getAllPieces());
}

inline const eval::EvalState& Board::evalState() const
{
    return m_EvalState;
}

inline Bitboard Board::checkers() const
{
    return m_State->checkInfo.checkers;
}

inline Bitboard Board::pinners(Color color) const
{
    return m_State->checkInfo.pinners[static_cast<int>(color)];
}

inline Bitboard Board::checkBlockers(Color color) const
{
    return m_State->checkInfo.blockers[static_cast<int>(color)];
}

inline int Board::seePieceValue(PieceType type) const
{
    return SEE_PIECE_VALUES[static_cast<int>(type) - 1];
}

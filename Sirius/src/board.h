#pragma once

#include "defs.h"
#include "bitboard.h"
#include "eval/psqt_state.h"
#include "zobrist.h"

#include <string_view>
#include <string>
#include <array>
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

    int halfMoveClock;
    int pliesFromNull;
    int epSquare;
    int castlingRights;
    int repetitions;
    int lastRepetition;
    ZKey zkey;
    ZKey pawnKey;
    CheckInfo checkInfo;
    Bitboard threats;
    eval::PsqtState psqtState;
};

class Board
{
public:
    static constexpr const char* defaultFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    Board();

    void setToFen(const std::string_view& fen);
    void setToEpd(const std::string_view& epd);

    std::string stringRep() const;
    std::string fenStr() const;
    std::string epdStr() const;

    void makeMove(Move move);
    void unmakeMove();
    void makeNullMove();
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

    Piece pieceAt(Square square) const;
    Bitboard pieces(PieceType type) const;
    Bitboard pieces(Color color, PieceType type) const;
    Bitboard pieces(Color color) const;
    Bitboard allPieces() const;
    Square kingSq(Color color) const;

    bool squareAttacked(Color color, Square square) const;
    bool squareAttacked(Color color, Square square, Bitboard blockers) const;
    Bitboard attackersTo(Color color, Square square) const;
    Bitboard attackersTo(Color color, Square square, Bitboard blockers) const;
    Bitboard attackersTo(Square square) const;
    Bitboard attackersTo(Square square, Bitboard blockers) const;

    bool isPassedPawn(Square square) const;
    bool isIsolatedPawn(Square square) const;

    Bitboard pinnersBlockers(Square square, Bitboard attackers, Bitboard& pinners) const;

    Bitboard checkers() const;
    Bitboard checkBlockers(Color color) const;
    Bitboard threats() const;

    bool see(Move move, int margin) const;
    bool isLegal(Move move) const;
    ZKey keyAfter(Move move) const;

    const eval::PsqtState& psqtState() const;
private:
    const BoardState& currState() const;
    BoardState& currState();
    Bitboard pinners(Color color) const;

    void updateCheckInfo();
    void calcThreats();
    void calcRepetitions();
    void addPiece(Square pos, Color color, PieceType piece);
    void addPiece(Square pos, Piece piece);
    void removePiece(Square pos);
    void movePiece(Square src, Square dst);

    int seePieceValue(PieceType type) const;

    static constexpr std::array<int, 6> SEE_PIECE_VALUES = {
        100, 450, 450, 675, 1300, 0
    };

    std::vector<BoardState> m_States;

    Color m_SideToMove;

    int m_GamePly;
};

inline const BoardState& Board::currState() const
{
    return m_States.back();
}

inline BoardState& Board::currState()
{
    return m_States.back();
}

inline bool Board::isDraw(int searchPly)
{
    return is50MoveDraw() || is3FoldDraw(searchPly);
}

inline bool Board::is3FoldDraw(int searchPly)
{
    return currState().repetitions > 1 || (currState().repetitions == 1 && currState().lastRepetition < searchPly);
}

inline bool Board::is50MoveDraw()
{
    return currState().halfMoveClock >= 100;
}

inline Color Board::sideToMove() const
{
    return m_SideToMove;
}

inline int Board::epSquare() const
{
    return currState().epSquare;
}

inline int Board::gamePly() const
{
    return m_GamePly;
}

inline int Board::halfMoveClock() const
{
    return currState().halfMoveClock;
}

inline int Board::pliesFromNull() const
{
    return currState().pliesFromNull;
}

inline int Board::castlingRights() const
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

inline Piece Board::pieceAt(Square square) const
{
    return currState().squares[square.value()];
}

inline Bitboard Board::pieces(PieceType type) const
{
    return currState().pieces[static_cast<int>(type)];
}

inline Bitboard Board::pieces(Color color, PieceType type) const
{
    return pieces(type) & pieces(color);
}

inline Bitboard Board::pieces(Color color) const
{
    return currState().colors[static_cast<int>(color)];
}

inline Bitboard Board::allPieces() const
{
    return currState().colors[0] | currState().colors[1];
}

inline Square Board::kingSq(Color color) const
{
    return pieces(color, PieceType::KING).lsb();
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

inline const eval::PsqtState& Board::psqtState() const
{
    return currState().psqtState;
}

inline Bitboard Board::checkers() const
{
    return currState().checkInfo.checkers;
}

inline Bitboard Board::pinners(Color color) const
{
    return currState().checkInfo.pinners[static_cast<int>(color)];
}

inline Bitboard Board::checkBlockers(Color color) const
{
    return currState().checkInfo.blockers[static_cast<int>(color)];
}

inline Bitboard Board::threats() const
{
    return currState().threats;
}

inline int Board::seePieceValue(PieceType type) const
{
    return SEE_PIECE_VALUES[static_cast<int>(type)];
}

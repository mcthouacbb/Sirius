#pragma once

#include <array>

#include "../../defs.h"
#include "../../board.h"
#include "../../bitboard.h"

namespace Eval {

using BB = uint64_t;

enum class Piece {
    pawn,
    knight,
    bishop,
    rook,
    queen,
    king,
};

enum class Color {
    white,
    black
};

inline Color operator~(Color color) { return static_cast<Color>(static_cast<uint8_t>(color) ^ 1); }

// clang-format off
enum class Square{
    a1, b1, c1, d1, e1, f1, g1, h1,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a8, b8, c8, d8, e8, f8, g8, h8
};
// clang-format on

constexpr Square flip_rank(Square s) {  // Swap A1 <-> A8
    return static_cast<Square>(static_cast<uint8_t>(s) ^ static_cast<uint8_t>(Square::a8));
}

inline Square bb_to_square(BB bb) {
    assert(bb != 0);
    return static_cast<Square>(std::countr_zero(bb));
}

inline BB square_to_bb(Square sq) { return static_cast<BB>(1) << static_cast<int>(sq); }

inline int rank(Square sq){ return static_cast<size_t>(sq) / 8;}
inline int file(Square sq){ return static_cast<size_t>(sq) % 8;}

class EvalPosition {
    std::array<BB, 2> colors;
    std::array<BB, 6> pieces;

   public:
    EvalPosition() = default;
    explicit EvalPosition(
      BB white, BB black, BB pawns, BB knights, BB bishops, BB rooks, BB queens, BB kings) :
        colors({white, black}),
        pieces({pawns, knights, bishops, rooks, queens, kings}) {}

    inline BB operator[](Piece piece) const { return pieces[static_cast<size_t>(piece)]; }
    inline BB operator[](Color color) const { return colors[static_cast<size_t>(color)]; }
	// this is deprecated in c++20
    // inline BB operator[](Color color, Piece piece) const { return (*this)[piece] & (*this)[color]; }
	inline BB coloredPieces(Color color, Piece piece) const { return (*this)[piece] & (*this)[color]; }

    inline BB all() const { return colors[0] | colors[1]; }

    inline bool pawn_passed(Color c, Square s) const {
        const auto siriusc              = static_cast<::Color>(c);
		const auto siriussq             = ::Square(static_cast<int>(s));
        // const auto sfsq             = static_cast<::Square>(s);

        // const auto passed_pawn_span = pawn_attack_span(sfc, sfsq) | forward_file_bb(sfc, sfsq);
		const auto passed_pawn_span = attacks::passedPawnMask(siriusc, siriussq).value();
        return (coloredPieces(~c, Piece::pawn) & passed_pawn_span) == 0;
    }
};

class BBIter {
    BB occ;

   public:
    BBIter(BB occ) :
        occ(occ) {}

    inline auto operator<=>(const BBIter&) const = default;

    inline BBIter operator++() {

        occ &= occ - 1;
        return *this;
    }

    inline Square operator*() { return bb_to_square(occ); }

    inline BBIter begin() { return *this; }

    static BBIter end() { return {0}; }
};


#ifdef EVAL_TUNING
float winningProbability(Value value);
float update_gradient(const EvalPosition& pos, float targetProbability, float learning_rate);
void  writeBaeParams();
#endif

int absolute_evaluate(const EvalPosition& pos);

inline EvalPosition toEvalPosition(const Board& board) {
    return EvalPosition(board.pieces(::Color::WHITE).value(), board.pieces(::Color::BLACK).value(), board.pieces(::PieceType::PAWN).value(), board.pieces(::PieceType::KNIGHT).value(),
                        board.pieces(::PieceType::BISHOP).value(), board.pieces(::PieceType::ROOK).value(), board.pieces(::PieceType::QUEEN).value(), board.pieces(::PieceType::KING).value());
}


inline int evaluate(const Board& board) {
    int result = absolute_evaluate(toEvalPosition(board));
    // result *= 25;  // to make the scaling be closer to what the classical eval does
    // result /= 10;
    // assert(abs(result) < VALUE_KNOWN_WIN);
    if (board.sideToMove() == ::Color::BLACK)
    {
        result = -result;
    }
    return result + 32;
}

/*Value absolute_evaluate(const EvalPosition& pos);

inline EvalPosition toEvalPosition(const Position& pos) {
    return EvalPosition(pos.pieces(WHITE), pos.pieces(BLACK), pos.pieces(PAWN), pos.pieces(KNIGHT),
                        pos.pieces(BISHOP), pos.pieces(ROOK), pos.pieces(QUEEN), pos.pieces(KING));
}

inline Value evaluate(const Position& pos) {
    Value result = absolute_evaluate(toEvalPosition(pos));
    result *= 25;  // to make the scaling be closer to what the classical eval does
    result /= 10;
    assert(abs(result) < VALUE_KNOWN_WIN);
    if (pos.side_to_move() == BLACK)
    {
        result = -result;
    }
    return result + Tempo;
}*/
}  // namespace Eval
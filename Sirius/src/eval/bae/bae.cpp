
#include <array>
#include <cassert>
#include <cstdlib>
#include <initializer_list>
#include <type_traits>
#include <vector>
#include <functional>
#include <cmath>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <sstream>


// sirius polyfill stuff
#include <bit>
int popcount(uint64_t bb) {
	return std::popcount(bb);
}

#include "bae.h"

#ifndef EVAL_TUNING
    #include "bae_params.h"
#endif

namespace Eval {

// sirius polyfill
using Bitboard = uint64_t;

enum class Phase : size_t {
    opening = 0,
    endgame = 1
};

template<typename ValueType>
struct BaeParamsSinglePhase {
    // clang-format off
    std::array<std::array<std::array<std::array<std::array<std::array<std::array<ValueType, 64>, 6>, 64>, 6> , 2> , 4>, 2> pieceRelativePst;
    std::array<std::array<ValueType, 19683>, 30> pawnStructureBonus;
    std::array<ValueType, 59049> pieceComboBonus;
    // clang-format on
};

template<typename ValueType>
class BaeParams {
    std::vector<BaeParamsSinglePhase<ValueType>> params =
      std::vector<BaeParamsSinglePhase<ValueType>>(2);

   public:
    BaeParamsSinglePhase<ValueType>& operator[](const Phase phase) {
        return params[static_cast<size_t>(phase)];
    }
    const BaeParamsSinglePhase<ValueType>& operator[](const Phase phase) const {
        return params[static_cast<size_t>(phase)];
    }


    void do_for_all(const std::function<void(ValueType&)>& op) {
        for (auto& singlePhase : params)
        {
            // clang-format off
            for(auto& a : singlePhase.pieceRelativePst)
            for(auto& b : a)
            for(auto& c : b)
            for(auto& d : c)
            for(auto& e : d)
            for(auto& f : e)
            for(auto& g : f)
            {
                op(g);
            }

            for(auto& a : singlePhase.pawnStructureBonus)
            for(auto& b : a)
            {
                op(b);
            }

            for(auto& a : singlePhase.pieceComboBonus)
            {
                op(a);
            }
            // clang-format on
        }
    }
};


[[nodiscard]] int16_t i16_from_hex(const char* const s) {
    const char hexString[] = {s[0], s[1], s[2], s[3], '\0'};
    uint16_t   tmp         = std::strtoul(hexString, nullptr, 16);
    return *reinterpret_cast<int16_t*>(&tmp);
}

#ifdef EVAL_TUNING
[[nodiscard]] std::string to_hex(const int16_t a) {
    const uint16_t     aUnsiged = *reinterpret_cast<const uint16_t*>(&a);
    std::ostringstream oss;
    oss << std::setw(4) << std::setfill('0') << std::hex << aUnsiged;
    std::string result = oss.str();
    assert(result.size() == 4);
    assert(i16_from_hex(result.c_str()) == a);
    return result;
}
#endif

#ifdef EVAL_TUNING
BaeParams<float> baeParams = []() {
    BaeParams<float> baeParams{};
    baeParams.do_for_all([](float& value) { value = 0.0; });
    return baeParams;
}();
#else
const BaeParams<int> baeParams = []() {
    BaeParams<int> baeParams{};

    size_t n = 0;

    baeParams.do_for_all([&](int& value) {
        value = static_cast<int>(i16_from_hex(&Eval::rawBaeContent[n]));
        n += 4;
    });

    return baeParams;
}();
#endif

class EvalValue {
   private:
    std::array<int, 2> value = {0, 0};

   public:
    int& operator[](const Phase phase) { return value[static_cast<size_t>(phase)]; }
};


struct EvalGradient {
    float gamePhaseFactor;
    float g;
};

template<typename T>
concept EvalState = std::same_as<T, EvalValue> || std::same_as<T, EvalGradient>;

#define ADD_VALUE(evalState, goodFor, param) \
    if constexpr (std::is_same_v<EvalGradient, std::remove_cvref_t<decltype(*(evalState))>>) \
    { \
        const float f = ((goodFor) == Color::black ? -1.0F : 1.0F) * (evalState)->g; \
        baeParams[Phase::opening].param += f * (evalState)->gamePhaseFactor; \
        baeParams[Phase::endgame].param += f * (1.0F - (evalState)->gamePhaseFactor); \
    } \
    else \
    { \
        for (Phase phase : {Phase::opening, Phase::endgame}) \
        { \
            int value = static_cast<int>(baeParams[phase].param); \
            if constexpr ((goodFor) == Color::black) \
            { \
                value = -value; \
            } \
            (*(evalState))[phase] += value; \
        } \
    }

Square color_conditional_mirror_vertically(const Square square, const Color color) {
    if (color == Color::black)
    {
        return flip_rank(square);
    }
    return square;
}

#define FOR_PIECE_RANGE(body) \
    if constexpr (ourPiece == Piece::pawn || ourPiece == Piece::king) \
    { \
        { \
            constexpr Piece otherPiece = Piece::pawn; \
            body \
        } \
        { \
            constexpr Piece otherPiece = Piece::knight; \
            body \
        } \
        { \
            constexpr Piece otherPiece = Piece::bishop; \
            body \
        } \
        { \
            constexpr Piece otherPiece = Piece::rook; \
            body \
        } \
        { \
            constexpr Piece otherPiece = Piece::queen; \
            body \
        } \
        { \
            constexpr Piece otherPiece = Piece::king; \
            body \
        } \
    } \
    if constexpr (ourPiece == Piece::queen) \
    { \
        { \
            constexpr Piece otherPiece = Piece::pawn; \
            body \
        } \
        { \
            constexpr Piece otherPiece = Piece::knight; \
            body \
        } \
        { \
            constexpr Piece otherPiece = Piece::bishop; \
            body \
        } \
        { \
            constexpr Piece otherPiece = Piece::rook; \
            body \
        } \
        { \
            constexpr Piece otherPiece = Piece::queen; \
            body \
        } \
    } \
    if constexpr (ourPiece == Piece::knight) \
    { \
        { \
            constexpr Piece otherPiece = Piece::pawn; \
            body \
        } \
        { \
            constexpr Piece otherPiece = Piece::knight; \
            body \
        } \
        { \
            constexpr Piece otherPiece = Piece::bishop; \
            body \
        } \
        { \
            constexpr Piece otherPiece = Piece::rook; \
            body \
        } \
    } \
    if constexpr (ourPiece == Piece::bishop) \
    { \
        { \
            constexpr Piece otherPiece = Piece::pawn; \
            body \
        } \
        { \
            constexpr Piece otherPiece = Piece::bishop; \
            body \
        } \
        { \
            constexpr Piece otherPiece = Piece::rook; \
            body \
        } \
    } \
    if constexpr (ourPiece == Piece::rook) \
    { \
        { \
            constexpr Piece otherPiece = Piece::pawn; \
            body \
        } \
        { \
            constexpr Piece otherPiece = Piece::rook; \
            body \
        } \
    }

template<Piece ourPiece, Color us, EvalState EvalState>
void piece_relative_pst(const EvalPosition& pos,
                        EvalState* const    evalState,
                        const Square        ourSquareIn) {

    const Square ourSquare = color_conditional_mirror_vertically(ourSquareIn, us);
    const Square enemyKingSquare =
      color_conditional_mirror_vertically(bb_to_square(pos.coloredPieces(~us, Piece::king)), us);
    const size_t roughEnemyKingFile = file(enemyKingSquare) / 2;
    const size_t roughEnemyKingRank = rank(enemyKingSquare) / 4;


    FOR_PIECE_RANGE({
        for (const size_t relativity : {0, 1})
        {
            for (const Square otherSquareIn : BBIter(pos.coloredPieces(relativity == 0 ? us : ~us, otherPiece)))
            {
                const Square otherSquare = color_conditional_mirror_vertically(otherSquareIn, us);

                ADD_VALUE(evalState, us,
                          pieceRelativePst[roughEnemyKingRank][roughEnemyKingFile][relativity]
                                          [static_cast<size_t>(ourPiece)][static_cast<size_t>(
                                            ourSquare)][static_cast<size_t>(otherPiece)]
                                          [static_cast<size_t>(otherSquare)])
            }
        }
    })
}

#undef FOR_PIECE_RANGE

template<Piece piece, Color color, EvalState EvalState>
void evaluate_piece(const EvalPosition& pos, EvalState* const evalState, const Square square) {
    if constexpr (piece == Piece::pawn)
    {
        if (pos.pawn_passed(color, square))
        {
            piece_relative_pst<Piece::pawn, color>(pos, evalState, square);
        }
    }
    else
    {
        piece_relative_pst<piece, color>(pos, evalState, square);
    }
}

template<Piece piece, Color color, EvalState EvalState>
void evaluate_piece_type_from_whites_perspective(const EvalPosition& pos,
                                                 EvalState* const    evalState) {

    for (const Square square : BBIter(pos.coloredPieces(color, piece)))
    {
        evaluate_piece<piece, color>(pos, evalState, square);
    }
}

template<Piece piece, EvalState EvalState>
void evaluate_piece_type_from_whites_perspective(const EvalPosition& pos,
                                                 EvalState* const    evalState) {

    evaluate_piece_type_from_whites_perspective<piece, Color::white>(pos, evalState);
    evaluate_piece_type_from_whites_perspective<piece, Color::black>(pos, evalState);
}

template<EvalState EvalState>
void evaluate_piece_type_from_whites_perspective(const EvalPosition& pos,
                                                 EvalState* const    evalState) {

    evaluate_piece_type_from_whites_perspective<Piece::pawn>(pos, evalState);
    evaluate_piece_type_from_whites_perspective<Piece::knight>(pos, evalState);
    evaluate_piece_type_from_whites_perspective<Piece::bishop>(pos, evalState);
    evaluate_piece_type_from_whites_perspective<Piece::rook>(pos, evalState);
    evaluate_piece_type_from_whites_perspective<Piece::queen>(pos, evalState);
    evaluate_piece_type_from_whites_perspective<Piece::king>(pos, evalState);
}

size_t pawn_mask_index(const EvalPosition& pos, const Square square) {
    const Bitboard whitePawns =
      pos.coloredPieces(Color::white, Piece::pawn) >> (static_cast<int>(square) - static_cast<int>(Square::b2));
    const Bitboard blackPawns =
      pos.coloredPieces(Color::black, Piece::pawn) >> (static_cast<int>(square) - static_cast<int>(Square::b2));

    size_t result  = 0;
    size_t counter = 1;

    for (const Bitboard bit : {
           square_to_bb(Square::a3),
           square_to_bb(Square::b3),
           square_to_bb(Square::c3),
           square_to_bb(Square::a2),
           square_to_bb(Square::b2),
           square_to_bb(Square::c2),
           square_to_bb(Square::a1),
           square_to_bb(Square::b1),
           square_to_bb(Square::c1),
         })
    {
        if ((whitePawns & bit) != 0)
        {
            result += counter * 2;
        }
        else if ((blackPawns & bit) != 0)
        {
            result += counter * 1;
        }

        counter *= 3;
    }

    return result;
}

template<EvalState EvalState>
void evaluate_3x3_pawn_structure_from_whites_perspective(const EvalPosition& pos,
                                                         EvalState* const    evalState) {
    for (const Square square : {
           Square::b3, Square::c3, Square::d3, Square::e3, Square::f3, Square::g3,
           Square::b4, Square::c4, Square::d4, Square::e4, Square::f4, Square::g4,
           Square::b5, Square::c5, Square::d5, Square::e5, Square::f5, Square::g5,
           Square::b6, Square::c6, Square::d6, Square::e6, Square::f6, Square::g6,
         })
    {
        const Bitboard mask3x3 =
          attacks::kingAttacks(::Square(static_cast<int>(square))).value() | square_to_bb(square);

        if (popcount(mask3x3 & pos[Piece::pawn]) >= 2)
        {
            const size_t index = pawn_mask_index(pos, square);
            assert(index < 19683);
            ADD_VALUE(evalState, Color::white,
                      pawnStructureBonus[(static_cast<size_t>(square)
                                          - static_cast<size_t>(Square::b3))][index]);
        }
    }
}

size_t piece_combo_index(const EvalPosition& pos) {
    size_t result  = 0;
    size_t counter = 1;
    for (const Color color : {Color::white, Color::black})
    {
        for (const Piece piece :
             {Piece::pawn, Piece::knight, Piece::bishop, Piece::rook, Piece::queen})
        {
            const size_t pieceCount = std::min(2, popcount(pos.coloredPieces(color, piece)));
            result += pieceCount * counter;
            counter *= 3;
        }
    }
    return result;
}

template<EvalState EvalState>
void piece_combo_bonus_white_perspective(const EvalPosition& pos, EvalState* const evalState) {
    if (std::max(popcount(pos.coloredPieces(Color::white, Piece::pawn)), popcount(pos.coloredPieces(Color::black, Piece::pawn)))
        <= 2)
    {
        const size_t index = piece_combo_index(pos);
        assert(index < 59049);
        ADD_VALUE(evalState, Color::white, pieceComboBonus[index]);
    }
}

#undef ADD_VALUE

template<EvalState EvalState>
void absolute_evaluate(const EvalPosition& pos, EvalState* const evalState) {
    evaluate_piece_type_from_whites_perspective(pos, evalState);
    evaluate_3x3_pawn_structure_from_whites_perspective(pos, evalState);
    piece_combo_bonus_white_perspective(pos, evalState);
}

int absolute_evaluate(const EvalPosition& pos) {
    EvalValue evalState{};
    absolute_evaluate(pos, &evalState);
    const int phase = popcount(pos.all());
    return (evalState[Phase::opening] * phase + evalState[Phase::endgame] * (32 - phase)) / 32;
}

#ifdef EVAL_TUNING
float error(const float outcome, const float estimate) {
    return std::pow(outcome - estimate, 2.0F);
}

float errorDerivative(const float outcome, const float estimate) {
    return 2.0F * (outcome - estimate);
}

constexpr float k = 400.0;


float winningProbabilityDerivative(const Value value) {
    return (std::log(10.0F) * std::pow(2.0F, -2.0F - ((static_cast<float>(value)) / k))
            * std::pow(5.0F, -((static_cast<float>(value)) / k)))
         / std::pow(1.0F + std::pow(10.0F, -((static_cast<float>(value)) / k)), 2.0F);
}
#endif


#ifdef EVAL_TUNING

void writeBaeParams() {
    std::ofstream out("src/bae_params.h");
    out << "#pragma once\nnamespace Eval { constexpr char rawBaeContent[] = R\"(";

    baeParams.do_for_all([&out](auto& value) { out << to_hex(static_cast<int16_t>(value)); });

    out << ")\"; }\n" << std::flush;
}

float winningProbability(const Value value) {
    return 1.0F / (1.0F + std::pow(10.0F, -((static_cast<float>(value)) / k)));
}

float update_gradient(const EvalPosition& pos,
                      const float         targetProbability,
                      const float         learning_rate) {
    const int   phase              = popcount(pos.all());
    const Value currentValue       = absolute_evaluate(pos);
    const float currentProbability = winningProbability(currentValue);
    const float currentError       = error(targetProbability, currentProbability);

    EvalGradient evalState{.gamePhaseFactor = static_cast<float>(phase) / 32.0F,
                           .g               = learning_rate
                              * errorDerivative(targetProbability, currentProbability)
                              * winningProbabilityDerivative(currentValue)};

    // std::cout << "targetValue: " << targetValue << std::endl;
    // std::cout << "currentValue: " << currentValue << std::endl;
    // std::cout << "targetProbability: " << targetProbability << std::endl;
    // std::cout << "currentProbability: " << currentProbability << std::endl;
    // std::cout << "currentError: " << currentError << std::endl;

    absolute_evaluate(pos, &evalState);

    return currentError;
}

#endif

}  // namespace Eval
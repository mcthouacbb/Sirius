#pragma once

#include "../board.h"
#include "eval_state.h"

namespace eval
{

constexpr int SCALE_FACTOR_NORMAL = 128;
constexpr int SCALE_FACTOR_DRAW = 0;

}

namespace eval::endgames
{

/*using EndgameFunc = int(const Board&, const EvalState&, Color);

enum class EndgameType
{
    EVAL,
    SCALE
};

struct Endgame
{
    Endgame()
        : func(nullptr), strongSide(Color::WHITE), key(0), type(EndgameType::EVAL)
    {
    }
    explicit Endgame(Color c, EndgameFunc* func, EndgameType type)
        : func(func), strongSide(c), key(0), type(type)
    {
    }

    int operator()(const Board& board, const EvalState& evalState) const
    {
        int result = (*func)(board, evalState, strongSide);
        return strongSide == board.sideToMove() || type == EndgameType::SCALE ? result : -result;
    }

    EndgameFunc* func;
    Color strongSide;
    uint64_t key;
    EndgameType type;
};

void init();
const Endgame* probeEvalFunc(const Board& board);
const Endgame* probeScaleFunc(const Board& board, Color strongSide);*/

}

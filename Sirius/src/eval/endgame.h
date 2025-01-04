#include "../board.h"
#include "eval_state.h"

namespace eval::endgames
{

using EndgameFunc = int(const Board&, const EvalState&, Color);

struct Endgame
{
    Endgame()
        : func(nullptr), strongSide(Color::WHITE), materialKey(0)
    {

    }
    explicit Endgame(Color c, EndgameFunc* func)
        : func(func), strongSide(c)
    {

    }

    int operator()(const Board& board, const EvalState& evalState) const
    {
        int result = (*func)(board, evalState, strongSide);
        return strongSide == board.sideToMove() ? result : -result;
    }

    EndgameFunc* func;
    Color strongSide;
    uint64_t materialKey;
};

void init();
const Endgame* probe(const Board& board);

}

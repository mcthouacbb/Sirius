#include "psqt_state.h"
#include "../board.h"

namespace eval
{

ScorePair PsqtState::evaluate(const Board& board) const
{
    return accumulators[static_cast<int>(Color::WHITE)].materialPsqt[0]
        + accumulators[static_cast<int>(Color::BLACK)].materialPsqt[0];
}

}

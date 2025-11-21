#include "psqt_state.h"
#include "../board.h"

namespace eval
{

ScorePair PsqtState::evaluate(const Board& board) const
{
    i32 whiteBucket = getKingBucket(board.kingSq(Color::WHITE));
    i32 blackBucket = getKingBucket(board.kingSq(Color::BLACK));
    return accumulators[static_cast<i32>(Color::WHITE)].materialPsqt[whiteBucket]
        + accumulators[static_cast<i32>(Color::BLACK)].materialPsqt[blackBucket];
}

}

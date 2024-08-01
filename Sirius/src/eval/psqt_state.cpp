#include "psqt_state.h"
#include "../board.h"

namespace eval
{

PackedScore PsqtState::evaluate(const Board& board) const
{
    int whiteBucket = getKingBucket(board.kingSq(Color::WHITE));
    int blackBucket = getKingBucket(board.kingSq(Color::BLACK));
    return
        accumulators[static_cast<int>(Color::WHITE)].materialPsqt[whiteBucket] +
        accumulators[static_cast<int>(Color::BLACK)].materialPsqt[blackBucket];
}


}

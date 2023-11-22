#pragma once

#include "board.h"
#include "movegen.h"
#include "history.h"
#include <array>

struct HistoryMove
{
    static constexpr int NO_HISTORY = 100000;

    Move move;
    int history;
};

bool moveIsQuiet(const Board& board, Move move);
bool moveIsCapture(const Board& board, Move move);

class MoveOrdering
{
public:
    static constexpr int QUIET_HISTORY_SCORE = 0;
    static constexpr int KILLER_SCORE = 65550;
    static constexpr int PROMOTION_SCORE = 65555;
    static constexpr int CAPTURE_SCORE = 65560;

    MoveOrdering(const Board& board, MoveList& moves, Move hashMove);
    MoveOrdering(const Board& board, MoveList& moves, Move hashMove, const std::array<Move, 2>& killers, const History& history);

    HistoryMove selectMove(uint32_t index);
private:
    MoveList& m_Moves;
    std::array<int, 256> m_MoveScores;
};

#pragma once

#include "board.h"
#include "movegen.h"
#include <array>

struct ExtMove
{
    Move move;
    int score;
};

bool moveIsQuiet(const Board& board, Move move);
bool moveIsCapture(const Board& board, Move move);

class MoveOrdering
{
public:
    static constexpr int HISTORY_MAX = 65536;
    static constexpr int KILLER_SCORE = 65550;
    static constexpr int PROMOTION_SCORE = 65555;
    static constexpr int CAPTURE_SCORE = 65560;

    MoveOrdering(const Board& board, MoveList& moves, Move hashMove);
    MoveOrdering(const Board& board, MoveList& moves, Move hashMove, std::array<Move, 2>& killers, std::array<int, 4096>& history);

    ExtMove selectMove(uint32_t index);
private:
    MoveList& m_Moves;
    std::array<int, 256> m_MoveScores;
};

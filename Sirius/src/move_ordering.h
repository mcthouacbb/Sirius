#pragma once

#include "board.h"
#include "movegen.h"
#include <array>

struct SearchStack;

struct ScoredMove
{
    Move move;
    i32 score;
};

bool moveIsQuiet(const Board& board, Move move);
bool moveIsCapture(const Board& board, Move move);

class MoveOrdering
{
public:
    static constexpr i32 NO_MOVE = -8000000;
    static constexpr i32 FIRST_KILLER_SCORE = 300001;
    static constexpr i32 SECOND_KILLER_SCORE = 300000;
    static constexpr i32 PROMOTION_SCORE = 400000;
    static constexpr i32 CAPTURE_SCORE = 500000;

    MoveOrdering(const Board&);

    ScoredMove selectMove();
    ScoredMove selectHighest();

private:
    i32 scoreMove(Move move) const;
    // i32 scoreQuiet(Move move) const;
    // i32 scoreMoveQSearch(Move move) const;

    const Board& m_Board;
    MoveList m_Moves;

    std::array<i32, 256> m_MoveScores;

    u32 m_Curr;
};

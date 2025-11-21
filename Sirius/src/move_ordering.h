#pragma once

#include "board.h"
#include "history.h"
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

enum class MovePickStage
{
    TT_MOVE,
    GEN_NOISY,
    GOOD_NOISY,
    FIRST_KILLER,
    SECOND_KILLER,
    GEN_QUIETS,
    BAD_NOISY_QUIETS,

    QS_TT_MOVE,
    QS_GEN_NOISIES,
    QS_NOISIES
};

inline MovePickStage operator++(MovePickStage& stage)
{
    assert(stage != MovePickStage::QS_NOISIES);
    stage = static_cast<MovePickStage>(static_cast<i32>(stage) + 1);
    return stage;
}

class MoveOrdering
{
public:
    static constexpr i32 NO_MOVE = -8000000;
    static constexpr i32 FIRST_KILLER_SCORE = 300001;
    static constexpr i32 SECOND_KILLER_SCORE = 300000;
    static constexpr i32 PROMOTION_SCORE = 400000;
    static constexpr i32 CAPTURE_SCORE = 500000;

    MoveOrdering(const Board& board, Move ttMove, const History& history);
    MoveOrdering(const Board& board, Move hashMove, const std::array<Move, 2>& killers,
        SearchStack* stack, i32 ply, const History& history);

    ScoredMove selectMove();
    ScoredMove selectHighest();

private:
    i32 scoreNoisy(Move move) const;
    i32 scoreQuiet(Move move) const;
    i32 scoreMoveQSearch(Move move) const;

    const Board& m_Board;
    MoveList m_Moves;
    Move m_TTMove;
    const History& m_History;
    SearchStack* m_Stack;
    i32 m_Ply;
    std::array<Move, 2> m_Killers;

    std::array<i32, 256> m_MoveScores;

    u32 m_Curr;
    u32 m_NoisyEnd;
    MovePickStage m_Stage;
};

#pragma once

#include "board.h"
#include "movegen.h"
#include "history.h"
#include <array>

struct ScoredMove
{
    Move move;
    int score;
};

bool moveIsQuiet(const Board& board, Move move);
bool moveIsCapture(const Board& board, Move move);

enum class MovePickStage
{
    TT_MOVE,
    GEN_NOISY,
    GOOD_NOISY,
    GEN_QUIETS,
    BAD_NOISY_QUIETS,

    QS_TT_MOVE,
    QS_GEN_NOISIES,
    QS_NOISIES
};

inline MovePickStage operator++(MovePickStage& stage)
{
    assert(stage != MovePickStage::QS_NOISIES);
    stage = static_cast<MovePickStage>(static_cast<int>(stage) + 1);
    return stage;
}

class MoveOrdering
{
public:
    static constexpr int NO_MOVE = -8000000;
    static constexpr int KILLER_SCORE = 300000;
    static constexpr int PROMOTION_SCORE = 400000;
    static constexpr int CAPTURE_SCORE = 500000;

    MoveOrdering(const Board& board, Move ttMove, const History& history);
    MoveOrdering(const Board& board, Move hashMove, const std::array<Move, 2>& killers, std::span<const CHEntry* const> contHistEntries, const History& history);

    ScoredMove selectMove();
    ScoredMove selectHighest();
private:
    int scoreNoisy(Move move) const;
    int scoreQuiet(Move move) const;
    int scoreMoveQSearch(Move move) const;

    const Board& m_Board;
    MoveList m_Moves;
    Move m_TTMove;
    const History& m_History;
    std::span<const CHEntry* const> m_ContHistEntries;
    std::array<Move, 2> m_Killers;

    std::array<int, 256> m_MoveScores;

    uint32_t m_Curr;
    uint32_t m_NoisyEnd;
    MovePickStage m_Stage;
};

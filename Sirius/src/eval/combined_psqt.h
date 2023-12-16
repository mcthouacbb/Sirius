#pragma once

#include "../defs.h"
#include "combined_psqt.h"
#include "eval_constants.h"

#include <array>

namespace eval
{

constexpr inline auto init()
{
    std::array<std::array<std::array<PackedScore, 64>, 7>, 2> combined = {};
    for (int piece = 1; piece < 7; piece++)
        for (int sq = 0; sq < 64; sq++)
        {
            combined[0][piece][sq] = (MATERIAL[piece] + PSQT[piece][sq ^ 0b111000]);
            combined[1][piece][sq] = -(MATERIAL[piece] + PSQT[piece][sq]);
        }
    return combined;
}

constexpr auto combinedPsqt = init();

inline PackedScore combinedPsqtScore(Color color, PieceType piece, int square)
{
    return combinedPsqt[static_cast<int>(color)][static_cast<int>(piece)][square];
}


}

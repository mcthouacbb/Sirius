#pragma once

#include "../defs.h"
#include "combined_psqt.h"
#include "eval_constants.h"

#include <array>

namespace eval
{

constexpr auto init()
{
    std::array<std::array<std::array<std::array<PackedScore, 64>, 6>, 2>, 2> combined = {};
    for (int bucket = 0; bucket < 2; bucket++)
        for (int piece = 0; piece < 6; piece++)
            for (int sq = 0; sq < 64; sq++)
            {
                int mirror = bucket * 0b111;
                combined[bucket][0][piece][sq] = (MATERIAL[piece] + PSQT[piece][sq ^ 0b111000 ^ mirror]);
                combined[bucket][1][piece][sq] = -(MATERIAL[piece] + PSQT[piece][sq ^ mirror]);
            }
    return combined;
}

constexpr auto combinedPsqt = init();

inline PackedScore combinedPsqtScore(int bucket, Color color, PieceType piece, int square)
{
    return combinedPsqt[bucket][static_cast<int>(color)][static_cast<int>(piece)][square];
}


}

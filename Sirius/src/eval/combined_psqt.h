#pragma once

#include "../defs.h"
#include "combined_psqt.h"
#include "eval_constants.h"
#include "../util/multi_array.h"

#include <array>

namespace eval
{

constexpr auto init()
{
    MultiArray<PackedScore, 2, 2, 6, 64> combined = {};
    for (int bucket = 0; bucket < 2; bucket++)
        for (int piece = 0; piece < 6; piece++)
            for (int sq = 0; sq < 64; sq++)
            {
                int mirror = bucket * 0b111;
                int mg = (MATERIAL_MG[piece] + PSQT_MG[piece][sq ^ 0b111000 ^ mirror]);
                int eg = (MATERIAL_EG[piece] + PSQT_EG[piece][sq ^ 0b111000 ^ mirror]);
                combined[bucket][0][piece][sq] = PackedScore(mg, eg);

                mg = -(MATERIAL_MG[piece] + PSQT_MG[piece][sq ^ mirror]);
                eg = -(MATERIAL_EG[piece] + PSQT_EG[piece][sq ^ mirror]);
                combined[bucket][1][piece][sq] = PackedScore(mg, eg);
            }
    return combined;
}

constexpr auto combinedPsqt = init();

inline PackedScore combinedPsqtScore(int bucket, Color color, PieceType piece, Square square)
{
    return combinedPsqt[bucket][static_cast<int>(color)][static_cast<int>(piece)][square.value()];
}


}

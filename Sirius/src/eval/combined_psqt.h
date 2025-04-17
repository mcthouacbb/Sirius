#pragma once

#include "../defs.h"
#include "../util/multi_array.h"
#include "combined_psqt.h"
#include "eval_constants.h"

#include <array>

namespace eval
{

constexpr auto initPsqt()
{
    MultiArray<ScorePair, 2, 2, 6, 64> combined = {};
    for (int bucket = 0; bucket < 2; bucket++)
        for (int piece = 0; piece < 6; piece++)
            for (int sq = 0; sq < 64; sq++)
            {
                int mirror = bucket * 0b111;

                int mg = (MATERIAL_MG[piece] + PSQT_MG[piece][sq ^ 0b111000 ^ mirror]);
                int eg = (MATERIAL_EG[piece] + PSQT_EG[piece][sq ^ 0b111000 ^ mirror]);
                combined[bucket][0][piece][sq] = ScorePair(mg, eg);

                mg = -(MATERIAL_MG[piece] + PSQT_MG[piece][sq ^ mirror]);
                eg = -(MATERIAL_EG[piece] + PSQT_EG[piece][sq ^ mirror]);
                combined[bucket][1][piece][sq] = ScorePair(mg, eg);
            }
    return combined;
}

constexpr auto combinedPsqt = initPsqt();

inline ScorePair combinedPsqtScore(int bucket, Color color, PieceType piece, Square square)
{
    return combinedPsqt[bucket][static_cast<int>(color)][static_cast<int>(piece)][square.value()];
}

}

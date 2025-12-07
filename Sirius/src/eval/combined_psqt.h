#pragma once

#include "../defs.h"
#include "../util/multi_array.h"
#include "combined_psqt.h"
#include "eval_constants.h"

#include <array>

namespace eval
{

/*constexpr auto initPsqt()
{
    MultiArray<ScorePair, 2, 2, 6, 64> combined = {};
    for (i32 bucket = 0; bucket < 2; bucket++)
        for (i32 piece = 0; piece < 6; piece++)
            for (i32 sq = 0; sq < 64; sq++)
            {
                i32 mirror = bucket * 0b111;
                combined[bucket][0][piece][sq] =
                    (MATERIAL[piece] + PSQT[piece][sq ^ 0b111000 ^ mirror]);
                combined[bucket][1][piece][sq] = -(MATERIAL[piece] + PSQT[piece][sq ^ mirror]);
            }
    return combined;
}*/

constexpr auto initPsqt()
{
    MultiArray<ScorePair, 2, 6, 64> combined = {};
    for (i32 piece = 0; piece < 6; piece++)
        for (i32 sq = 0; sq < 64; sq++)
        {
            // white
            combined[0][piece][sq] = (MATERIAL[piece] + PSQT[piece][sq ^ 0b111000]);
            // black
            combined[1][piece][sq] = -(MATERIAL[piece] + PSQT[piece][sq]);
        }
    return combined;
}

constexpr auto combinedPsqt = initPsqt();

/*inline ScorePair combinedPsqtScore(i32 bucket, Color color, PieceType piece, Square square)
{
    return combinedPsqt[bucket][static_cast<i32>(color)][static_cast<i32>(piece)][square.value()];
}*/

inline ScorePair combinedPsqtScore(Color color, PieceType piece, Square square)
{
    return combinedPsqt[static_cast<i32>(color)][static_cast<i32>(piece)][square.value()];
}

}

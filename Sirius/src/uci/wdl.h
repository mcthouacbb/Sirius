#pragma once

#include "../board.h"
#include <algorithm>
#include <cmath>

namespace uci
{
constexpr f64 as[] = {-420.12493420, 1155.97899531, -1074.82879152, 441.44515956};
constexpr f64 bs[] = {-106.59407292, 300.77010455, -267.28444080, 114.30281819};
constexpr i32 NormalizeToPawnValue = 102;

inline i32 normalizedScore(i32 raw)
{
    if (isMateScore(raw))
        return raw;
    return raw * 100 / NormalizeToPawnValue;
}

inline f64 expectedWinRate(f64 a, f64 b, i32 score)
{
    return 1.0 / (1.0 + std::exp(-(static_cast<f64>(score) - a) / b));
}

struct WDL
{
    f32 winProb;
    f32 lossProb;
};

inline WDL expectedWDL(const Board& board, i32 score)
{
    i32 material = board.pieces(PieceType::PAWN).popcount()
        + 3 * (board.pieces(PieceType::KNIGHT) | board.pieces(PieceType::BISHOP)).popcount()
        + 5 * board.pieces(PieceType::ROOK).popcount() + 9 * board.pieces(PieceType::QUEEN).popcount();

    f64 mom = static_cast<f64>(std::clamp(material, 17, 78)) / 58.0;

    f64 a = ((as[0] * mom + as[1]) * mom + as[2]) * mom + as[3];
    f64 b = ((bs[0] * mom + bs[1]) * mom + bs[2]) * mom + bs[3];

    WDL result = {};
    result.winProb = expectedWinRate(a, b, score);
    result.lossProb = expectedWinRate(a, b, -score);
    return result;
}

}

#pragma once

#include "../board.h"
#include <algorithm>
#include <cmath>

namespace uci
{

constexpr double as[] = {-412.68396633, 1138.34624335, -1064.76420906, 441.46636437};
constexpr double bs[] = {-109.72871711, 309.87996679, -275.78524440, 116.98051856};
constexpr int NormalizeToPawnValue = 102;

constexpr int normalizedScore(int raw)
{
    if (isMateScore(raw))
        return raw;
    return raw * 100 / NormalizeToPawnValue;
}

double expectedWinRate(double a, double b, int score)
{
    return 1.0 / (1.0 + std::exp(-(static_cast<double>(score) - a) / b));
}

struct WDL
{
    float winProb;
    float lossProb;
};

WDL expectedWDL(const Board& board, int score)
{
    int material = board.pieces(PieceType::PAWN).popcount()
        + 3 * (board.pieces(PieceType::KNIGHT) | board.pieces(PieceType::BISHOP)).popcount()
        + 5 * board.pieces(PieceType::ROOK).popcount() + 9 * board.pieces(PieceType::QUEEN).popcount();

    double mom = static_cast<double>(std::clamp(material, 17, 78)) / 58.0;

    double a = ((as[0] * mom + as[1]) * mom + as[2]) * mom + as[3];
    double b = ((bs[0] * mom + bs[1]) * mom + bs[2]) * mom + bs[3];

    WDL result = {};
    result.winProb = expectedWinRate(a, b, score);
    result.lossProb = expectedWinRate(a, b, -score);
    return result;
}

}

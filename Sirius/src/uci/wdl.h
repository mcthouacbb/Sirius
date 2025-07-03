#pragma once

#include "../board.h"
#include <algorithm>
#include <cmath>

namespace uci
{
constexpr double as[] = {-420.12493420, 1155.97899531, -1074.82879152, 441.44515956};
constexpr double bs[] = {-106.59407292, 300.77010455, -267.28444080, 114.30281819};
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

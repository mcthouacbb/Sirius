#pragma once

#include "../defs.h"
#include "multi_array.h"

inline std::array<PieceType, 8> scharnaglBackrank(uint32_t index)
{
    std::array<PieceType, 8> result{};
    result.fill(PieceType::NONE);

    // 4 locations for the light square and dark square bishop
    uint32_t darkSqBishop = index % 4;
    index /= 4;
    uint32_t lightSqBishop = index % 4;
    index /= 4;

    result[darkSqBishop * 2] = PieceType::BISHOP;
    result[lightSqBishop * 2 + 1] = PieceType::BISHOP;

    const auto nthFreeSq = [&](uint32_t n)
    {
        uint32_t j = 0;
        for (uint32_t i = 0; i < result.size(); i++)
        {
            if (result[i] == PieceType::NONE)
            {
                if (j == n)
                    return i;
                j++;
            }
        }
        throw std::runtime_error("nthFreeSq did not find a free square");
    };

    // 6 locations for the queen
    uint32_t queen = index % 6;
    index /= 6;

    result[nthFreeSq(queen)] = PieceType::QUEEN;

    // 5 choose 2 locations for the knights
    constexpr MultiArray<uint32_t, 10, 2> knightIndices = {
        {{0, 1}, {0, 2}, {0, 3}, {0, 4}, {1, 2}, {1, 3}, {1, 4}, {2, 3}, {2, 4}, {3, 4}}};

    auto knights = knightIndices[index];
    uint32_t knight0 = nthFreeSq(knights[0]);
    uint32_t knight1 = nthFreeSq(knights[1]);

    result[knight0] = PieceType::KNIGHT;
    result[knight1] = PieceType::KNIGHT;

    // remaining 3 squares must be RKR, as king must be between rooks
    result[nthFreeSq(0)] = PieceType::ROOK;
    result[nthFreeSq(0)] = PieceType::KING;
    result[nthFreeSq(0)] = PieceType::ROOK;
    return result;
}

inline std::string scharnaglBackrankFen(uint32_t index)
{
    constexpr std::array<char, 8> pieceTypeChars = {'P', 'N', 'B', 'R', 'Q', 'K'};
    auto backrankPieces = scharnaglBackrank(index);
    std::string result(8, ' ');
    for (uint32_t i = 0; i < 8; i++)
        result[i] = pieceTypeChars[static_cast<uint32_t>(backrankPieces[i])];
    return result;
}

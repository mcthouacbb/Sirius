#pragma once

#include "defs.h"
#include <array>

namespace cuckoo
{

inline std::array<uint64_t, 8192> keyDiffs;
inline std::array<Move, 8192> moves;

void init();

constexpr uint64_t H1(uint64_t keyDiff)
{
    return keyDiff % 8192;
}

constexpr uint64_t H2(uint64_t keyDiff)
{
    return (keyDiff >> 16) % 8192;
}

}

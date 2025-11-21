#pragma once

#include "defs.h"
#include <array>

namespace cuckoo
{

inline std::array<u64, 8192> keyDiffs;
inline std::array<Move, 8192> moves;

void init();

constexpr u64 H1(u64 keyDiff)
{
    return keyDiff % 8192;
}

constexpr u64 H2(u64 keyDiff)
{
    return (keyDiff >> 16) % 8192;
}

}

#pragma once

#include <cstdint>

constexpr uint64_t murmurHash3(uint64_t key)
{
    key ^= key >> 33;
    key *= 0xff51afd7ed558ccd;
    key ^= key >> 33;
    key *= 0xc4ceb9fe1a85ec53;
    key ^= key >> 33;
    return key;
};

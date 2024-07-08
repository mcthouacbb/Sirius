#pragma once

#include <cstdint>

// based off of https://stackoverflow.com/a/27160892/20789760

struct PRNG
{
public:
    constexpr PRNG() = default;
    constexpr void seed(uint64_t s);
    constexpr uint64_t next64();
private:
    uint64_t a, b, c, counter;
};

constexpr void PRNG::seed(uint64_t s)
{
    a = b = c = s;
    counter = 1;
    for (int i = 0; i < 12; i++)
        next64();
}

constexpr uint64_t PRNG::next64()
{
    uint64_t tmp = a + b + counter++;
    a = b ^ (b >> 12);
    b = c + (c << 13);
    c = ((c << 25) | (c >> (64 - 25))) + tmp;
    return tmp;
}

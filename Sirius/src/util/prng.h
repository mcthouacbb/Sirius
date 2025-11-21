#pragma once

#include <cstdint>

// based off of https://stackoverflow.com/a/27160892/20789760

struct PRNG
{
public:
    constexpr PRNG() = default;
    constexpr void seed(u64 s);
    constexpr u64 next64();

private:
    u64 a, b, c, counter;
};

constexpr void PRNG::seed(u64 s)
{
    a = b = c = s;
    counter = 1;
    for (i32 i = 0; i < 12; i++)
        next64();
}

constexpr u64 PRNG::next64()
{
    u64 tmp = a + b + counter++;
    a = b ^ (b >> 12);
    b = c + (c << 13);
    c = ((c << 25) | (c >> (64 - 25))) + tmp;
    return tmp;
}

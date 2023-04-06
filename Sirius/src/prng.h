#pragma once

#include <cstdint>

// based off of https://stackoverflow.com/a/27160892/20789760

struct PRNG
{
public:
	void seed(uint64_t s);
	uint64_t next64();
private:
	uint64_t a, b, c, counter;
};
inline void PRNG::seed(uint64_t s)
{
	a = b = c = s;
	counter = 1;
	for (int i = 0; i < 12; i++)
		next64();
}
inline uint64_t PRNG::next64()
{
	uint64_t tmp = a + b + counter++;
	a = b ^ (b >> 12);
	b = c + (c << 13);
	c = ((c << 25) | (c >> (64 - 25))) + tmp;
	return tmp;
}
/*static uint64_t rng_a, rng_b, rng_c, rng_counter;
uint64_t rng64() {
    uint64_t tmp = rng_a + rng_b + rng_counter++;
    rng_a = rng_b ^ (rng_b >> 12);
    rng_b = rng_c + (rng_c << 3);
    rng_c = ((rng_c << 25) | (rng_c >> (64-25))) + tmp;
    return tmp;
}
void seed(uint64_t s) {
    rng_a = rng_b = rng_c = s; rng_counter = 1;
    for (int i = 0; i < 12; i++) rng64();
}*/
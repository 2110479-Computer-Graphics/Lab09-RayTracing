#pragma once

#include <cstdint>

struct Rng {
  uint64_t state = 0;
  uint64_t inc = 1;

  Rng(uint64_t seed, uint64_t stream) {
    inc = (stream << 1u) | 1u;
    next_u32();
    state += seed;
    next_u32();
  }

  uint32_t next_u32() {
    uint64_t old = state;
    state = old * 6364136223846793005ULL + inc;
    uint32_t xorshifted = uint32_t(((old >> 18u) ^ old) >> 27u);
    uint32_t rot = uint32_t(old >> 59u);
    return (xorshifted >> rot) | (xorshifted << ((32u - rot) & 31u));
  }

  float next() { return float(next_u32() >> 8) * (1.0f / 16777216.0f); }
};

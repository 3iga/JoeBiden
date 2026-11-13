#pragma once
#include "MathSupport.hpp"
#include <cstdint>
#include <random>
#include <vector>

struct Random {
std::mt19937 engine;
explicit Random(std::uint32_t seed):engine(seed) {
}
// Avoid implementation-dependent std::uniform_real_distribution sequences.
// A 53-bit draw keeps very rare encounter probabilities independent of frame rate.
double unitDouble() {
    const std::uint64_t high = std::uint64_t(engine() >> 5);
    const std::uint64_t low = std::uint64_t(engine() >> 6);
    return double(high * 67108864ULL + low) / 9007199254740992.0;
}
float unit() {
    return float(engine() >> 8) * (1.0f / 16777216.0f);
}
float range(float lo, float hi) {
    return lo+(hi-lo)*unit();
}
int integer(int lo, int hi) {
    return lo+int(unit()*float(hi-lo+1));
}
};

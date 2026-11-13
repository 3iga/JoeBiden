#pragma once
#include "MathSupport.hpp"
#include <cstdint>
#include <random>
#include <vector>


struct Random {
    std::mt19937 engine;
    explicit Random(std::uint32_t seed):engine(seed){}
    // Avoid implementation-dependent std::uniform_real_distribution sequences.
    float unit() { return float(engine() >> 8) * (1.0f / 16777216.0f); }
    float range(float lo,float hi) { return lo+(hi-lo)*unit(); }
    int integer(int lo,int hi) { return lo+int(unit()*float(hi-lo+1)); }
};

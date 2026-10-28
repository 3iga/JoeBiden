#pragma once
#include "Math.hpp"
#include <cstdint>
#include <random>
#include <vector>

namespace reef {
struct Random {
    std::mt19937 engine;
    explicit Random(std::uint32_t seed):engine(seed){}
    // Avoid implementation-dependent std::uniform_real_distribution sequences.
    float unit() { return float(engine() >> 8) * (1.0f / 16777216.0f); }
    float range(float lo,float hi) { return lo+(hi-lo)*unit(); }
    int integer(int lo,int hi) { return lo+int(unit()*float(hi-lo+1)); }
};
inline float seabedHeight(float x,float z) {
    return .18f*std::sin(.32f*x)+.15f*std::cos(.29f*z)+.12f*std::sin(.16f*x+.25f*z);
}
inline bool reservedSpace(float x,float z) {
    // A sandy dive lane and a clear area around the transform-demo coral.
    float dx=x+1.8f,dz=z+2.6f;
    return (std::abs(x)<1.6f && z>-14) || dx*dx+dz*dz<6.8f;
}
inline std::vector<Vec2> scatter(std::uint32_t seed,int target,float minDistance) {
    // Poisson-disc-style dart throwing: randomized candidates, a hard spacing test.
    // O(n^2) is intentionally easy to inspect and sufficient for this small reef.
    Random rng(seed);std::vector<Vec2> points;
    for(int attempt=0;attempt<target*500 && int(points.size())<target;++attempt) {
        Vec2 p{rng.range(-16,16),rng.range(-23,6)};
        if(reservedSpace(p.x,p.y)) continue;
        bool clear=true;
        for(Vec2 q:points) {
            float x=p.x-q.x,z=p.y-q.y;
            if(x*x+z*z<minDistance*minDistance) { clear=false;break; }
        }
        if(clear) points.push_back(p);
    }
    return points; // Actual accepted count is shown in the UI, not assumed.
}
} // namespace reef

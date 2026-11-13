#pragma once
#include "MathSupport.hpp"
#include <cstdint>
#include <utility>

constexpr double ChunkSize = 24.0;
constexpr int CoralVariants = 10;
using ChunkKey = std::pair<std::int64_t, std::int64_t>;

// Unsigned arithmetic deliberately wraps: reproducible hash, including negative coordinates.
inline std::uint64_t hash64(std::uint64_t x) {
    x += 0x9e3779b97f4a7c15ULL;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
    x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
    return x ^ (x >> 31);
}
inline std::uint32_t chunkSeed(ChunkKey k, std::uint32_t seed) {
    return static_cast<std::uint32_t>(hash64(static_cast<std::uint64_t>(k.first)) ^
    hash64(static_cast<std::uint64_t>(k.second) + 0xa1785aULL) ^ hash64(seed));
}
inline double smooth(double t) {
    return t*t*(3.0-2.0*t);
}
inline double noiseValue(std::int64_t x, std::int64_t z, std::uint32_t seed) {
    return double(chunkSeed({x, z}, seed)) / 4294967295.0;
}
inline double valueNoise(double x, double z, std::uint32_t seed) {
    auto ix=static_cast<std::int64_t>(std::floor(x));
    auto iz=static_cast<std::int64_t>(std::floor(z));
    double a=smooth(x-double(ix)), b=smooth(z-double(iz));
    double n00=noiseValue(ix, iz, seed), n10=noiseValue(ix+1, iz, seed);
    double n01=noiseValue(ix, iz+1, seed), n11=noiseValue(ix+1, iz+1, seed);
    return (n00*(1-a)+n10*a)*(1-b)+(n01*(1-a)+n11*a)*b;
}
inline float terrainHeight(double x, double z, std::uint32_t seed) {
    // Continuous global height field. Every neighboring tile samples the same border.
    return float((valueNoise(x*.024, z*.024, seed)-.5)*3.2 +
    (valueNoise(x*.085, z*.085, seed+17)-.5)*.65 +
    .09*std::sin(x*.6+z*.18));
}
inline Vec3 chunkOffset(ChunkKey k, ChunkKey origin) {
    return {float(double(k.first-origin.first)*ChunkSize), 0,
    float(double(k.second-origin.second)*ChunkSize)};
}
inline bool sphereInRange(Vec3 center, float radius, Vec3 eye, float range) {
    const Vec3 d=center-eye;
    const float limit=range+radius;
    return dot(d, d)<=limit*limit;
}
inline float smooth01(float a, float b, float v) {
    float t=clamp((v-a)/(b-a), 0, 1);
    return t*t*(3-2*t);
}
struct Frustum {
Vec4 plane[6]{};
explicit Frustum(const Mat4& clip) {
    const float* m = glm::value_ptr(clip);
    for (int axis=0; axis<3; ++axis) for (int side=0; side<2; ++side) {
        float sign=side==0?1.0f:-1.0f;
        Vec4 p{m[3]+sign*m[axis], m[7]+sign*m[4+axis],
        m[11]+sign*m[8+axis], m[15]+sign*m[12+axis]};
        float n=std::sqrt(p.x*p.x+p.y*p.y+p.z*p.z);
        if (n>0) {
            p.x/=n;
            p.y/=n;
            p.z/=n;
            p.w/=n;
        }
        plane[axis*2+side]=p;
    }
}
bool containsSphere(Vec3 c, float r) const {
for (auto p:plane) if (p.x*c.x+p.y*c.y+p.z*c.z+p.w < -r) return false;
return true;
}
};

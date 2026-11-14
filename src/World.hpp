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
inline float smooth01(float a, float b, float v);
inline float mountainRelief(double x, double z, std::uint32_t seed) {
    constexpr double cellSize=520.0;
    const auto cellX=static_cast<std::int64_t>(std::floor(x/cellSize));
    const auto cellZ=static_cast<std::int64_t>(std::floor(z/cellSize));
    float relief=0;
    for (std::int64_t dz=-1; dz<=1; ++dz)for (std::int64_t dx=-1; dx<=1; ++dx) {
        const std::int64_t gx=cellX+dx, gz=cellZ+dz;
        if (noiseValue(gx, gz, seed+401u)>.49)continue;
        const double centerX=(double(gx)+.16+.68*noiseValue(gx, gz, seed+409u))*cellSize;
        const double centerZ=(double(gz)+.16+.68*noiseValue(gx, gz, seed+419u))*cellSize;
        const double radius=62.0+92.0*noiseValue(gx, gz, seed+431u);
        const double distance=std::sqrt((x-centerX)*(x-centerX)+(z-centerZ)*(z-centerZ));
        const double t=distance/radius;
        if (t>=1.0)continue;
        const float height=float(38.0+78.0*noiseValue(gx, gz, seed+443u));
        const float profile=float(std::pow(1.0-t*t, .90));
        relief=std::max(relief, height*profile);
    }
    return relief;
}
inline float canyonRelief(double x, double z, std::uint32_t seed) {
    constexpr double cellSize=520.0;
    const auto cellX=static_cast<std::int64_t>(std::floor(x/cellSize));
    const auto cellZ=static_cast<std::int64_t>(std::floor(z/cellSize));
    float depth=0;
    for (std::int64_t dz=-1; dz<=1; ++dz)for (std::int64_t dx=-1; dx<=1; ++dx) {
        const std::int64_t gx=cellX+dx, gz=cellZ+dz;
        if (noiseValue(gx, gz, seed+503u)>.06)continue;
        const double centerX=(double(gx)+.5)*cellSize;
        const double centerZ=(double(gz)+.5)*cellSize;
        const double angle=noiseValue(gx, gz, seed+509u)*6.283185307179586;
        const double deltaX=x-centerX, deltaZ=z-centerZ;
        const double along=deltaX*std::cos(angle)+deltaZ*std::sin(angle);
        const double across=-deltaX*std::sin(angle)+deltaZ*std::cos(angle);
        const double phase=noiseValue(gx, gz, seed+521u)*6.283185307179586;
        const double bend=32.0*std::sin(along*.009+phase)+12.0*std::sin(along*.027+phase*1.7);
        const double halfWidth=12.0+16.0*noiseValue(gx, gz, seed+523u);
        const double sideDistance=std::abs(across-bend);
        if (sideDistance>=halfWidth)continue;
        const float crossSection=1.0f-smooth01(.12f, 1.0f, float(sideDistance/halfWidth));
        const float endFade=1.0f-smooth01(float(cellSize*.36), float(cellSize*.5), float(std::abs(along)));
        const float canyonDepth=float(28.0+52.0*noiseValue(gx, gz, seed+541u));
        depth=std::max(depth, canyonDepth*crossSection*endFade);
    }
    return depth;
}
inline float craterRelief(double x, double z, std::uint32_t seed) {
    constexpr double cellSize=620.0;
    const auto cellX=static_cast<std::int64_t>(std::floor(x/cellSize));
    const auto cellZ=static_cast<std::int64_t>(std::floor(z/cellSize));
    float depression=0, rimHeight=0;
    for (std::int64_t dz=-1; dz<=1; ++dz)for (std::int64_t dx=-1; dx<=1; ++dx) {
        const std::int64_t gx=cellX+dx, gz=cellZ+dz;
        if (gx!=0||gz!=0)if (noiseValue(gx, gz, seed+601u)>.46)continue;
        const double centerX=(double(gx)+.5)*cellSize+(noiseValue(gx, gz, seed+607u)-.5)*150.0;
        const double centerZ=(double(gz)+.5)*cellSize+(noiseValue(gx, gz, seed+613u)-.5)*150.0;
        const double radius=76.0+86.0*noiseValue(gx, gz, seed+617u);
        const double t=std::sqrt((x-centerX)*(x-centerX)+(z-centerZ)*(z-centerZ))/radius;
        if (t>=1.30)continue;
        const float bowl=1.0f-smooth01(.08f, .88f, float(t));
        const float rim=std::exp(-float((t-.96)*(t-.96))/.016f);
        const float depth=float(95.0+95.0*noiseValue(gx, gz, seed+619u));
        const float lip=float(12.0+18.0*noiseValue(gx, gz, seed+631u));
        depression=std::max(depression, depth*bowl);
        rimHeight=std::max(rimHeight, lip*rim);
    }
    return rimHeight-depression;
}
inline float originalTerrainHeight(double x, double z, std::uint32_t seed) {
    // Continuous global bathymetry: broad plains and ridges plus deep trenches and craters.
    const float startupFade=smooth01(24.0f, 48.0f, float(std::sqrt(x*x+z*z)));
    const float broad=(float(valueNoise(x*.004, z*.004, seed+17u))-.5f)*14.0f;
    const float rolling=(float(valueNoise(x*.018, z*.018, seed+29u))-.5f)*5.0f;
    return float((valueNoise(x*.024, z*.024, seed)-.5)*3.2 + broad+rolling+
    (valueNoise(x*.085, z*.085, seed+37u)-.5)*.65 +
    .09*std::sin(x*.6+z*.18))+
    startupFade*(mountainRelief(x, z, seed)-canyonRelief(x, z, seed)+craterRelief(x, z, seed));
}
// Course topics: continuous procedural surfaces and seeded scalar noise.
// A deliberately composed opening valley makes the geometry visible immediately.
// Global coordinates, not per-tile random heights, keep all shared edges continuous.
inline float terrainHeight(double x, double z, std::uint32_t seed, bool enhanced = true) {
    const float original = originalTerrainHeight(x, z, seed);
    if (!enhanced) return original;
    const double distance = std::sqrt(x*x + z*z);
    const float local = 1.0f - smooth01(65.0f, 110.0f, float(distance));
    const double lane = -1.5 + 2.4 * std::sin((z + 5.0) * .065);
    const double left = (x + 14.0) / 8.5, right = (x - 16.0) / 10.0;
    const double along = (z + 19.0) / 28.0;
    const float banks = float(10.5 * std::exp(-left*left - along*along) +
                             13.0 * std::exp(-right*right - (z+28.0)*(z+28.0)/1100.0));
    const float back = float(6.0 * std::exp(-x*x/550.0 - (z+48.0)*(z+48.0)/100.0));
    const float channel = float(1.7 * std::exp(-(x-lane)*(x-lane)/26.0 - (z+16.0)*(z+16.0)/1400.0));
    const float smallRidges = float((valueNoise(x*.055, z*.055, seed+921u)-.5)*2.2);
    return original + local*(banks + back - channel) + smallRidges;
}
inline float reefGardenWeight(double x, double z, std::uint32_t seed) {
    const float patch = float(valueNoise(x*.070, z*.070, seed+947u));
    const float regional = .32f + .68f*smooth01(.24f, .70f,
        float(valueNoise(x*.009, z*.009, seed+953u)));
    const double lane = -1.5 + 2.4*std::sin((z+5.0)*.065);
    const float opening = 1.0f-smooth01(42.0f, 80.0f, float(std::sqrt(x*x+z*z)));
    const float clearLane = (1.0f-smooth01(1.6f, 4.5f, float(std::abs(x-lane))))*opening;
    return (.24f + .76f*smooth01(.24f, .66f, patch))*regional*(1.0f-.94f*clearLane);
}
inline int reefPalette(double x, double z, std::uint32_t seed) {
    // Regional palettes, rather than independently selecting every color of the rainbow.
    const float n = float(valueNoise(x*.019, z*.019, seed+971u));
    return n < .39f ? 0 : (n < .62f ? 1 : 2);
}
inline float reefIntensity(double x, double z, std::uint32_t seed) {
    const float patch=float(valueNoise(x*.003, z*.003, seed+701u));
    const float distantReef=smooth01(.67f, .84f, patch);
    const float starter=1.0f-smooth01(35.0f, 105.0f, float(std::sqrt(x*x+z*z)));
    return std::max(distantReef*.88f, starter*.68f);
}
inline float algaeIntensity(double x, double z, std::uint32_t seed) {
    const float patch=float(valueNoise(x*.0022, z*.0022, seed+719u));
    const float kelpBed=smooth01(.63f, .81f, patch);
    const float starter=1.0f-smooth01(42.0f, 115.0f, float(std::sqrt(x*x+z*z)));
    return std::max(kelpBed*.76f, starter*.46f);
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

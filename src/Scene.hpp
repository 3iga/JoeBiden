#pragma once
#include "MeshData.hpp"
#include "Procedural.hpp"
#include "Species.hpp"
#include "World.hpp"
#include <array>
#include <map>
#include <vector>

struct Camera {
Vec3 position{9.6f, 5.0f, 13.5f};
float yaw=radians(-121), pitch=radians(-13), fov=radians(58);
Vec3 forward() const { return safeNormalize({std::cos(yaw)*std::cos(pitch), std::sin(pitch), std::sin(yaw)*std::cos(pitch)});
}
Vec3 right() const { return safeNormalize(cross(forward(), {0, 1, 0}));
}
Vec3 up() const { return safeNormalize(cross(right(), forward()));
}
Mat4 view() const { return glm::lookAt(position, position + forward(), Vec3(0, 1, 0));
}
void lookToward(Vec3 target) {
    auto d=safeNormalize(target-position);
    yaw=std::atan2(d.z, d.x);
    pitch=std::asin(clamp(d.y, -1, 1));
}
void reset() {
    position={9.6f, 5, 13.5f};
    lookToward({-1.8f, 1.45f, -4.8f});
    fov=radians(58);
}
};
struct Settings {
bool lighting=true, caustics=true, textures=true, normalMaps=true, fog=true;
bool paused=false, wireframe=false, tour=false, ui=true, vsync=true, bubbles=true;
bool dayCycle=true;
bool visualUpgrade=true; // F3: rebuild the same seeded world with/without the visual pass.
float hour=10.5f, daySeconds=240, fogStart=14, visibility=64;
float swimSpeed=4.0f, fishSpeed=1.0f, lightAzimuth=0;
int quality=1, transformMode=1;
std::uint32_t seed=1337;
};
struct Transform {
Vec3 position{-1.8f, .32f, -2.6f}, rotation{}, size{1.15f, 1.15f, 1.15f};
Mat4 matrix() const {return compose(position, rotation, size);
}
};
enum class ObjectKind { Coral, Rock, Kelp, Rockslide, Structure, Seagrass, Debris, Arch };
enum class StructureVariant { Ruin, CargoShip, OceanLiner, AircraftCarrier, Aircraft, LexingtonCarrier, Submarine, FishingBoat };
constexpr int StructureVariantCount=8;
constexpr std::uint32_t StructurePickBase=0x40000000u;
struct StructureInfo {
const char* name;
const char* className;
const char* sinking;
const char* site;
const char* discovery;
const char* depth;
const char* context;
const char* source;
const char* source2;
};
const StructureInfo& structureInfo(int variant);
struct Object {
ObjectKind kind=ObjectKind::Coral;
int variant=0;
std::uint32_t pickId=0;
Transform transform;
Vec4 tint{1, 1, 1, 1};
Vec3 boundCenter{};
float radius=1;
};
struct Chunk {
ChunkKey key{};
MeshData terrain;
std::vector<Object> objects;
int coralCount=0;
Vec3 terrainCenter{}, boundsCenter{};
float terrainRadius=210, boundsRadius=210;
};
struct Fish {
int species=0, variant=0;
std::uint32_t id=0;
Vec3 position{}, velocity{}, target{}, home{};
float scale=1, lengthCm=10, massKg=.1f, timer=0;
float yaw=0, pitch=0, roll=0, tailAngle=0, finAngle=0, phase=0;
Vec4 tint{1, 1, 1, 1};
};
struct Lighting {
Vec3 direction{.3f, .9f, .3f}, ambient{.3f, .4f, .45f}, color{1, .94f, .8f};
Vec3 fogColor{.035f, .25f, .29f};
float daylight=1;
};
enum class SurveyState { Ready, Active, Complete, TimeUp };
struct Survey {
SurveyState state = SurveyState::Ready;
float remaining = 180.0f;
};
struct Megalodon {
bool active = false;
bool manuallySpawned = false;
Vec3 position{}, velocity{}, target{};
float age = 0, yaw = 0, pitch = 0, tailAngle = 0;
};
struct Scene {
Settings settings;
Survey survey;
Megalodon megalodon;
bool showMegalodonInfo=false, showHelp=false, showStructureInfo=false;
Camera camera;
Transform featured;
// Coordinates relative to original chunk (0,0).
ChunkKey origin{};
// Floating origin, in whole chunks (64-bit).
std::map<ChunkKey, Chunk> chunks;
std::vector<Fish> fish;
std::vector<Vec4> bubbles;
std::array<bool, SpeciesCount> discovered{};
Lighting light;
double time=0, tourTime=0;
std::uint64_t generation=0;
int pendingChunks=0, selectedFish=-1;
std::uint32_t selectedStructureId=0;
bool showFishInfo=false;
int guideSpecies=-1;
double worldX() const {return double(origin.first)*ChunkSize+camera.position.x;
}
double worldZ() const {return double(origin.second)*ChunkSize+camera.position.z;
}
float floorAt(Vec3 local) const {return terrainHeight(double(origin.first)*ChunkSize+local.x, double(origin.second)*ChunkSize+local.z, settings.seed, settings.visualUpgrade);
}
int discoveries() const {int n=0;
for (bool x:discovered) if (x)++n;
return n;
}
const Object* selectedStructure() const {
    if (!selectedStructureId)return nullptr;
    for (const auto& item:chunks)for (const auto& object:item.second.objects)
        if (object.kind==ObjectKind::Structure&&object.pickId==selectedStructureId)return &object;
    return nullptr;
}
bool selectedStructureLocation(double& x, double& z) const {
    for (const auto& item:chunks)for (const auto& object:item.second.objects)
        if (object.kind==ObjectKind::Structure&&object.pickId==selectedStructureId) {
            x=double(item.first.first)*ChunkSize+object.transform.position.x;
            z=double(item.first.second)*ChunkSize+object.transform.position.z;
            return true;
        }
    return false;
}
};
Chunk generateChunk(ChunkKey key, std::uint32_t seed, int quality, bool enhanced = true);

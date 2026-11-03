#pragma once
#include "Graphics.hpp"
#include "Procedural.hpp"
#include <array>
namespace reef {
struct Camera {
    Vec3 position{9.6f,5.0f,13.5f};
    float yaw=radians(-121.0f),pitch=radians(-13.0f),fov=radians(58);
    Vec3 forward() const { return normalize(Vec3(std::cos(yaw)*std::cos(pitch),std::sin(pitch),std::sin(yaw)*std::cos(pitch))); }
    Vec3 right() const { return normalize(cross(forward(),Vec3(0,1,0))); }
    Vec3 up() const { return normalize(cross(right(),forward())); }
    Mat4 view() const { return lookAt(position,position+forward()); }
    void lookToward(Vec3 target) { Vec3 d=normalize(target-position);yaw=std::atan2(d.z,d.x);pitch=std::asin(clamp(d.y,-1,1)); }
    void reset() { position={9.6f,5.0f,13.5f};lookToward({-1.8f,1.45f,-4.8f});fov=radians(58); }
};
struct Settings {
    bool lighting=true,caustics=true,textures=true,normalMaps=true,fog=true;
    bool paused=false,wireframe=false,tour=false,ui=true,vsync=true;
    bool bubbles=true;
    int quality=1,transformMode=1;
    std::uint32_t seed=1337;
};
struct Transform {
    Vec3 position{-1.8f,.32f,-2.6f},rotation{0,0,0},size{1.15f,1.15f,1.15f};
    Mat4 matrix() const { return compose(position,rotation,size); }
    void reset() { position={-1.8f,seabedHeight(-1.8f,-2.6f)+.32f,-2.6f};rotation=Vec3(0);size=Vec3(1.15f); }
};
struct Node {
    Mat4 local=Mat4::identity();
    Mesh* mesh=nullptr;
    Material* material=nullptr;
    Vec4 tint{1,1,1,1};
    std::vector<Node> children;
};
struct FishInfo {
    int index=-1,species=0,lengthCm=0;
    const char* name="";
    const char* trait="";
    float weightKg=0,scale=1,yaw=0;
    Vec3 position{};
    Vec4 color{1,1,1,1};
};
class Scene {
    Shader shader,background,bubbleShader;
    Mesh sand,branch,tube,plate,rock,kelp,sphere,tail,fin,ring;
    Material sandMaterial,coralMaterial,rockMaterial,fishMaterial,leafMaterial,plainMaterial;
    Texture causticTexture;
    Node fish;
    GLuint emptyVAO=0,bubbleVAO=0,bubbleVBO=0;
    std::vector<Vec4> bubbleOrigins;
    std::array<bool,4> discoveredFish{};
    int coralCount=0,fishCount=24;
    void material(const Material& m);
    void draw(Mesh& mesh,Material& material,const Mat4& model,Vec4 tint,float sway=0,float emission=0,int fishPattern=-1);
    void drawInstances(Mesh& mesh,Material& material,float sway=0);
    void drawNode(const Node& node,const Mat4& parent,int fishPattern=-1,float emission=0);
public:
    Transform featured;
    int drawCalls=0,triangleCount=0;
    explicit Scene(const std::filesystem::path& shaderPath);
    ~Scene();
    void regenerate(const Settings& settings);
    void render(const Camera& camera,const Settings& settings,float time,int width,int height,int selectedFish=-1);
    FishInfo fishInfo(int index,float time) const;
    int pickFish(const Camera& camera,float mouseX,float mouseY,int width,int height,float time) const;
    void discoverFish(int index);
    int discoveredSpeciesCount() const;
    int corals() const { return coralCount+1; }
    int fishes() const { return fishCount; }
};
} // namespace reef

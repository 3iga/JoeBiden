#pragma once
#include "Scene.hpp"

enum class Action { Lighting,Caustics,Textures,Normals,Fog,Pause,Wireframe,UI,Tour,Vsync,
    NewSeed,Quality,Home,Focus,ResetCoral,MoveMode,RotateMode,ScaleMode,StartSurvey,RestartSurvey,SpawnMegalodon,DismissMegalodon,Help,Count };
struct InputFrame {
    std::array<bool,static_cast<std::size_t>(Action::Count)> action{};
    Vec3 move{},transformAxis{};
    Vec2 mouseDelta{};
    float scroll=0;
    bool fast=false,looking=false;
    bool pressed(Action a) const {return action[static_cast<std::size_t>(a)];}
    void press(Action a) {action[static_cast<std::size_t>(a)]=true;}
};
class Update {
    Random random_{1337};
    Random encounterRandom_{0x4d454741u};
    std::uint32_t lastSeed_=0,nextFishId_=1;
    int lastQuality_=-1;
    void stream(Scene& scene,int budget);
    void rebase(Scene& scene);
    void spawnFish(Scene& scene,int index,bool nearStart=false);
    void animateFish(Scene& scene,float dt);
    void animateBubbles(Scene& scene,float dt);
    void animateMegalodon(Scene& scene, float dt);
public:
    void initialize(Scene& scene);
    void step(Scene& scene,const InputFrame& input,float dt);
    void regenerate(Scene& scene);
    void returnHome(Scene& scene,bool focus=false);
    void resetCoral(Scene& scene);
    void selectFish(Scene& scene,int index);
    static Lighting lighting(float hour);
    void startSurvey(Scene& scene);
    void resetSurvey(Scene& scene);
    void spawnMegalodon(Scene& scene, bool manual = true);
    static double encounterProbability(double seconds);
};

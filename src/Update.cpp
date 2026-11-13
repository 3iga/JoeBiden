#include "Update.hpp"
#include <algorithm>
#include <cmath>
#include <limits>

void Update::resetCoral(Scene& s) {
    s.featured={};
    s.featured.position.y=terrainHeight(-1.8, -2.6, s.settings.seed)+.25f;
}
void Update::initialize(Scene& s) {
    s.camera.reset();
    resetCoral(s);
    regenerate(s);
    s.light=lighting(s.settings.hour);
}
void Update::regenerate(Scene& s) {
    s.chunks.clear();
    ++s.generation;
    s.selectedFish=-1;
    s.showFishInfo=false;
    s.showStructureInfo=false;
    s.selectedStructureId=0;
    s.megalodon = {};
    s.showMegalodonInfo = false;
    lastSeed_=s.settings.seed;
    lastQuality_=s.settings.quality;
    random_=Random(s.settings.seed+71);
    const int counts[]={24, 40, 56};
    s.fish.resize(counts[std::max(0, std::min(s.settings.quality, 2))]);
    for (int i=0; i<int(s.fish.size()); ++i)spawnFish(s, i, true);
    s.bubbles.clear();
    for (int i=0; i<120; ++i)s.bubbles.push_back({s.camera.position.x+random_.range(-22, 22),
    s.camera.position.y+random_.range(-4, 8), s.camera.position.z+random_.range(-22, 22), random_.range(.02f, .055f)});
    stream(s, 1000);
    // One bounded initial load. Later movement generates only 3 tiles per frame.
}
void Update::returnHome(Scene& s, bool focus) {
    s.origin={0, 0};
    s.camera.reset();
    s.settings.tour=false;
    regenerate(s);
    if (focus) {
        s.camera.position=s.featured.position+Vec3(4.2f, 2.8f, 5.5f);
        s.camera.lookToward(s.featured.position+Vec3(0, 1, 0));
    }
}
void Update::rebase(Scene& s) {
    if (std::abs(s.camera.position.x)<48&&std::abs(s.camera.position.z)<48)return;
    auto dx=static_cast<std::int64_t>(std::floor((double(s.camera.position.x)+12)/ChunkSize));
    auto dz=static_cast<std::int64_t>(std::floor((double(s.camera.position.z)+12)/ChunkSize));
    Vec3 shift{float(double(dx)*ChunkSize), 0, float(double(dz)*ChunkSize)};
    s.origin.first+=dx;
    s.origin.second+=dz;
    s.camera.position=s.camera.position-shift;
    for (auto& f:s.fish) {
        f.position=f.position-shift;
        f.target=f.target-shift;
        f.home=f.home-shift;
    }
    for (auto& b:s.bubbles) {
        b.x-=shift.x;
        b.z-=shift.z;
    }
    if (s.megalodon.active) {
        s.megalodon.position -= shift;
        s.megalodon.target -= shift;
    }
}
void Update::stream(Scene& s, int budget) {
    const auto cx=s.origin.first+static_cast<std::int64_t>(std::floor((double(s.camera.position.x)+12)/ChunkSize));
    const auto cz=s.origin.second+static_cast<std::int64_t>(std::floor((double(s.camera.position.z)+12)/ChunkSize));
    const int radius=int(std::ceil(s.settings.visibility/float(ChunkSize)))+1;
    for (auto it=s.chunks.begin(); it!=s.chunks.end(); ) {
        if (std::abs(it->first.first-cx)>radius||std::abs(it->first.second-cz)>radius)it=s.chunks.erase(it);
        else ++it;
    }
    std::vector<ChunkKey> missing;
    for (int z=-radius; z<=radius; ++z)for (int x=-radius; x<=radius; ++x) {
        ChunkKey k{cx+x, cz+z};
        if (!s.chunks.count(k))missing.push_back(k);
    }
    std::sort(missing.begin(), missing.end(), [&](ChunkKey a, ChunkKey b) {
        auto ax=a.first-cx, az=a.second-cz, bx=b.first-cx, bz=b.second-cz;
        auto da=ax*ax+az*az, db=bx*bx+bz*bz;
        return da==db?a<b:da<db;
    });
    int n=std::min(budget, int(missing.size()));
    for (int i=0; i<n; ++i)s.chunks.emplace(missing[std::size_t(i)], generateChunk(missing[std::size_t(i)], s.settings.seed, s.settings.quality));
    s.pendingChunks=int(missing.size())-n;
    if (s.showStructureInfo&&!s.selectedStructure()) {
        s.showStructureInfo=false;
        s.selectedStructureId=0;
    }
}
Lighting Update::lighting(float hour) {
    const float angle = (hour - 6.0f) * 2.0f * Pi / 24.0f;
    const float elevation = std::sin(angle);
    const float day = smooth01(-.20f, .24f, elevation);
    const float twilight = 1.0f - smooth01(.03f, .55f, std::abs(elevation));
    Lighting light;
    light.daylight = day;
    Vec3 sun = safeNormalize(Vec3(std::cos(angle) * .7f, std::max(.08f, elevation), .35f));
    Vec3 moon = safeNormalize(Vec3(-std::cos(angle) * .7f, std::max(.08f, -elevation), -.35f));
    light.direction = safeNormalize(mix(moon, sun, day));
    light.ambient = mix(Vec3(.018f, .035f, .07f), Vec3(.29f, .40f, .43f), day);
    Vec3 daylight = mix(Vec3(1, .94f, .80f), Vec3(1, .46f, .22f), twilight * .65f);
    light.color = mix(Vec3(.10f, .16f, .32f), daylight, day);
    light.fogColor = mix(Vec3(.006f, .019f, .044f), Vec3(.035f, .25f, .285f), day);
    return light;
}
void Update::spawnFish(Scene& s, int index, bool nearStart) {
    Fish f;
    f.species=index%SpeciesCount;
    const auto& species=SpeciesList[std::size_t(f.species)];
    f.id=nextFishId_++;
    f.variant=index%3;
    f.phase=random_.range(0, 2*Pi);
    const float fraction=random_.range(.57f, .95f);
    f.lengthCm=species.maximumLengthCm*fraction;
    f.massKg=species.modelMassKg*fraction*fraction*fraction;
    // Small fish enlarged for legibility; this is NOT a 1:1 biological scale.
    f.scale=clamp(.42f+f.lengthCm*.020f, .50f, 2.4f);
    f.tint=species.color;
    float tint=random_.range(.83f, 1.08f);
    f.tint.x=clamp(f.tint.x*tint, 0, 1);
    f.tint.y=clamp(f.tint.y*tint, 0, 1);
    f.tint.z=clamp(f.tint.z*tint, 0, 1);
    Vec3 center=nearStart?s.camera.position+s.camera.forward()*14.0f:s.camera.position;
    float a=random_.range(0, 2*Pi), r=random_.range(3, nearStart?18.0f:s.settings.visibility*.78f);
    f.position=center+Vec3(std::cos(a)*r, random_.range(-2, 2), std::sin(a)*r);
    // Keep the opening view readable; never initially place a fish against the lens.
    Vec3 fromCamera = f.position - s.camera.position;
    fromCamera.y = 0;
    if (length(fromCamera) < 8) {
        Vec3 away = length(fromCamera) > .01f ? safeNormalize(fromCamera) : safeNormalize(Vec3(s.camera.forward().x, 0, s.camera.forward().z));
        f.position.x = s.camera.position.x + away.x * 8;
        f.position.z = s.camera.position.z + away.z * 8;
    }
    float floor=s.floorAt(f.position);
    f.position.y=std::max(floor+random_.range(1.8f, 5.5f), s.camera.position.y+random_.range(-3.4f, 1.5f));
    f.home=f.position;
    f.target=f.position+Vec3(std::cos(a)*3, 0, std::sin(a)*3);
    f.velocity=safeNormalize(f.target-f.position)*species.speed;
    f.timer=random_.range(.3f, 3);
    f.yaw=std::atan2(-f.velocity.z, f.velocity.x);
    s.fish[std::size_t(index)]=f;
    if (s.selectedFish==index)s.selectedFish=-1;
}
void Update::animateFish(Scene& s, float dt) {
    // Read an immutable snapshot so steering does not depend on vector iteration order.
    const auto previous=s.fish;
    for (int i=0; i<int(s.fish.size()); ++i) {
        Fish& f=s.fish[std::size_t(i)];
        const auto& species=SpeciesList[std::size_t(f.species)];
        if (length(f.position-s.camera.position)>s.settings.visibility+16) {
            spawnFish(s, i);
            continue;
        }
        f.timer-=dt;
        float dist=length(f.target-f.position);
        if (f.timer<=0||dist<1) {
            Vec3 anchor=species.behaviour==Behaviour::Hover?f.home:f.position;
            float radius=species.behaviour==Behaviour::Hover?3.5f:random_.range(4, 10);
            float a=random_.range(0, 2*Pi);
            f.target=anchor+Vec3(std::cos(a)*radius, random_.range(-1.2f, 1.2f), std::sin(a)*radius);
            if (species.behaviour==Behaviour::Graze)f.target.y=std::max(s.floorAt(f.target)+2.0f, s.camera.position.y-3.0f);
            f.target.y=std::max(f.target.y, s.floorAt(f.target)+1.5f);
            f.timer=random_.range(2.2f, 6.0f);
        }
        Vec3 direction=safeNormalize(f.target-f.position);
        Vec3 separation{}, alignment{}, cohesion{};
        int neighbors=0;
        for (std::size_t j=0; j<previous.size(); ++j)if (j!=std::size_t(i)) {
            Vec3 away=f.position-previous[j].position;
            float d=length(away);
            if (d<1.8f&&d>.001f)separation+=away*((1.8f-d)/(d*d));
            if (species.behaviour==Behaviour::Shoal&&previous[j].species==f.species&&d<10) {
                alignment+=previous[j].velocity;
                cohesion+=previous[j].position;
                ++neighbors;
            }
        }
        direction+=separation*.75f;
        if (neighbors) {
            direction+=safeNormalize(alignment)*.30f+safeNormalize(cohesion/float(neighbors)-f.position)*.28f;
        }
        // Conservative local obstacle avoidance (not a physics or collision solver).
        auto key=ChunkKey{s.origin.first+static_cast<std::int64_t>(std::floor((f.position.x+12)/24)),
        s.origin.second+static_cast<std::int64_t>(std::floor((f.position.z+12)/24))};
        for (int z=-1; z<=1; ++z)for (int x=-1; x<=1; ++x) {
            auto it=s.chunks.find({key.first+x, key.second+z});
            if (it==s.chunks.end())continue;
            Vec3 offset=chunkOffset(it->first, s.origin);
            for (const auto& o:it->second.objects)if (o.kind!=ObjectKind::Kelp&&o.kind!=ObjectKind::Seagrass) {
                Vec3 c=offset+o.boundCenter, away=f.position-c;
                float d=length(away);
                float r=o.kind==ObjectKind::Coral?o.radius*.53f+.45f:o.radius+.35f;
                if (d<r&&d>.001f)direction+=away/d*((r-d)*1.8f)+Vec3(0, .30f, 0);
            }
        }
        float floor=s.floorAt(f.position);
        if (f.position.y<floor+1.8f)direction.y+=2*(floor+1.8f-f.position.y);
        float speed=species.speed*s.settings.fishSpeed*(.88f+.12f*std::sin(float(s.time)*.6f+f.phase));
        float turn=species.behaviour==Behaviour::Heavy?1.0f:2.1f;
        Vec3 desired=safeNormalize(direction)*speed;
        f.velocity=mix(f.velocity, desired, 1-std::exp(-turn*dt));
        f.position+=f.velocity*dt;
        f.position.y=std::max(f.position.y, s.floorAt(f.position)+.8f);
        if (length(f.velocity)>.001f) {
            f.yaw=std::atan2(-f.velocity.z, f.velocity.x);
            f.pitch=std::atan2(f.velocity.y, std::sqrt(f.velocity.x*f.velocity.x+f.velocity.z*f.velocity.z));
        }
        f.roll=.035f*std::sin(float(s.time)*1.6f+f.phase);
        f.tailAngle=std::sin(float(s.time)*(5+species.speed*2)+f.phase)*.45f;
        f.finAngle=std::sin(float(s.time)*6+f.phase)*.18f;
    }
}
void Update::animateBubbles(Scene& s, float dt) {
    for (auto& b:s.bubbles) {
        b.y+=dt*(.30f+b.w*5);
        b.x+=dt*.03f*std::sin(float(s.time)+b.z);
        Vec3 p{b.x, b.y, b.z};
        if (length(p-s.camera.position)>s.settings.visibility||b.y>s.camera.position.y+11) {
            b.x=s.camera.position.x+random_.range(-24, 24);
            b.z=s.camera.position.z+random_.range(-24, 24);
            b.y=std::max(s.floorAt({b.x, 0, b.z})+.1f, s.camera.position.y-7);
        }
    }
}
void Update::selectFish(Scene& s, int index) {
    s.showStructureInfo=false;
    s.selectedStructureId=0;
    if (index == int(s.fish.size()) && s.megalodon.active) {
        s.showMegalodonInfo = true;
        s.selectedFish = -1;
        return;
    }
    if (index<0||index>=int(s.fish.size())) {
        s.selectedFish=-1;
        return;
    }
    s.selectedFish=index;
    int species=s.fish[std::size_t(index)].species;
    if (s.survey.state == SurveyState::Active) s.discovered[std::size_t(species)] = true;
    s.guideSpecies=species;
    s.showFishInfo=true;
    s.settings.tour=false;
    if (s.survey.state == SurveyState::Active && s.discoveries() == SpeciesCount)
    s.survey.state = SurveyState::Complete;
}
void Update::selectStructure(Scene& s, std::uint32_t id) {
    s.selectedStructureId=id;
    if (!s.selectedStructure()) {
        s.selectedStructureId=0;
        s.showStructureInfo=false;
        return;
    }
    s.selectedFish=-1;
    s.showFishInfo=false;
    s.showMegalodonInfo=false;
    s.showStructureInfo=true;
    s.settings.tour=false;
}
void Update::step(Scene& s, const InputFrame& in, float dt) {
    dt=std::isfinite(dt)?clamp(dt, 0, .25f):0;
    auto& o=s.settings;
    if (in.pressed(Action::StartSurvey)) startSurvey(s);
    if (in.pressed(Action::RestartSurvey)) resetSurvey(s);
    if (in.pressed(Action::Help)) {
        s.showHelp = !s.showHelp;
        o.ui = true;
    }
    if (in.pressed(Action::DismissMegalodon)) {
        s.megalodon.active=false;
        s.showMegalodonInfo=false;
    }
    o.quality=std::max(0, std::min(o.quality, 2));
    o.visibility=clamp(o.visibility, 24, 96);
    o.fogStart=clamp(o.fogStart, 0, o.visibility-3);
    o.daySeconds=clamp(o.daySeconds, 20, 1800);
    o.fishSpeed=clamp(o.fishSpeed, .05f, 3);
    o.swimSpeed=clamp(o.swimSpeed, 1, 20);
    auto toggle=[&](Action a, bool& value) {
        if (in.pressed(a))value=!value;
    };
    toggle(Action::Lighting, o.lighting);
    toggle(Action::Caustics, o.caustics);
    toggle(Action::Textures, o.textures);
    toggle(Action::Normals, o.normalMaps);
    toggle(Action::Fog, o.fog);
    toggle(Action::Pause, o.paused);
    toggle(Action::Wireframe, o.wireframe);
    toggle(Action::UI, o.ui);
    toggle(Action::Tour, o.tour);
    toggle(Action::Vsync, o.vsync);
    if (in.pressed(Action::NewSeed))++o.seed;
    if (in.pressed(Action::Quality))o.quality=(o.quality+1)%3;
    if (in.pressed(Action::ResetCoral))resetCoral(s);
    if (in.pressed(Action::MoveMode))o.transformMode=0;
    if (in.pressed(Action::RotateMode))o.transformMode=1;
    if (in.pressed(Action::ScaleMode))o.transformMode=2;
    if (in.pressed(Action::Home))returnHome(s, false);
    if (in.pressed(Action::Focus))returnHome(s, true);
    if (lastSeed_!=o.seed||lastQuality_!=o.quality) {
        resetCoral(s);
        regenerate(s);
    }
    Vec3 axis=in.transformAxis;
    if (o.transformMode==0)s.featured.position+=axis*dt*1.6f;
    if (o.transformMode==1)s.featured.rotation+=axis*dt*.9f;
    if (o.transformMode==2)s.featured.size+=axis*dt*.6f;
    for (int i=0; i<3; ++i)s.featured.size[i]=clamp(s.featured.size[i], .2f, 3);
    if (in.looking&&(in.mouseDelta.x!=0||in.mouseDelta.y!=0)) {
        s.camera.yaw+=in.mouseDelta.x*.0022f;
        s.camera.pitch=clamp(s.camera.pitch-in.mouseDelta.y*.0022f, radians(-85), radians(85));
        o.tour=false;
    }
    s.camera.fov=clamp(s.camera.fov-in.scroll*radians(2), radians(30), radians(85));
    Vec3 movement=s.camera.right()*in.move.x+Vec3(0, 1, 0)*in.move.y+s.camera.forward()*in.move.z;
    if (length(movement)>.01f) {
        o.tour=false;
        s.camera.position+=safeNormalize(movement)*dt*o.swimSpeed*(in.fast?2.5f:1);
    }
    if (!o.paused) {
        s.time+=dt;
        if (o.dayCycle)o.hour=std::fmod(o.hour+dt*24/o.daySeconds, 24.0f);
        if (o.tour)s.tourTime+=dt;
    }
    o.hour=std::fmod(std::max(o.hour, 0.0f), 24.0f);
    s.light=lighting(o.hour);
    s.light.direction = Vec3(rotateY(radians(o.lightAzimuth)) * Vec4(s.light.direction, 0));
    if (o.tour) {
        // Deliberately scripted camera tour; fish motion is independent steering.
        if (s.origin!=ChunkKey{0, 0}) {
            returnHome(s, false);
            o.tour=true;
        }
        float a=.55f+float(s.tourTime)*.08f;
        s.camera.position={std::sin(a)*16, 5.4f+.5f*std::sin(float(s.tourTime)*.12f), -5+std::cos(a)*20};
        s.camera.lookToward({-1.8f, 1.45f, -4.8f});
    }
    rebase(s);
    s.camera.position.y=std::max(s.camera.position.y, s.floorAt(s.camera.position)+.65f);
    stream(s, 3);
    if (in.pressed(Action::SpawnMegalodon)) spawnMegalodon(s, true);
    if (!o.paused) {
        animateFish(s, dt);
        animateBubbles(s, dt);
        animateMegalodon(s, dt);
        if (s.survey.state == SurveyState::Active) {
            s.survey.remaining = std::max(0.0f, s.survey.remaining - dt);
            if (s.survey.remaining == 0) s.survey.state = SurveyState::TimeUp;
        }
    }
}

void Update::startSurvey(Scene& s) {
    if (s.survey.state == SurveyState::Active) return;
    s.discovered.fill(false);
    s.survey = {SurveyState::Active, 180.0f};
    s.settings.paused = false;
}
void Update::resetSurvey(Scene& s) {
    s.discovered.fill(false);
    s.survey = {};
    s.selectedFish = -1;
    s.showFishInfo = false;
    returnHome(s, false);
}
double Update::encounterProbability(double seconds) {
    // A Poisson event: mean waiting time 10,000 unpaused minutes, NOT one roll/frame.
    return seconds > 0 ? -std::expm1(-seconds / 600000.0) : 0.0;
}
void Update::spawnMegalodon(Scene& s, bool manual) {
    Megalodon& shark = s.megalodon;
    shark = {};
    shark.active = true;
    shark.manuallySpawned = manual;
    Vec3 forward = safeNormalize(Vec3(s.camera.forward().x, 0, s.camera.forward().z));
    Vec3 right = s.camera.right();
    shark.position = s.camera.position + forward * 18.0f - right * 7.0f + Vec3(0, 2, 0);
    shark.position.y = std::max(shark.position.y, s.floorAt(shark.position) + 5.0f);
    shark.target = s.camera.position + forward * 18.0f + right * 22.0f + Vec3(0, 2, 0);
    shark.velocity = right * 3.0f;
    shark.yaw = std::atan2(-shark.velocity.z, shark.velocity.x);
}
void Update::animateMegalodon(Scene& s, float dt) {
    auto& shark = s.megalodon;
    if (!shark.active) {
        if (encounterRandom_.unitDouble() < encounterProbability(dt)) spawnMegalodon(s, false);
        return;
    }
    shark.age += dt;
    if (shark.age >= 90.0f || length(shark.position - s.camera.position) > s.settings.visibility + 40) {
        shark.active = false;
        return;
    }
    if (length(shark.target - shark.position) < 4.0f) {
        float angle = encounterRandom_.range(0, 2 * Pi);
        shark.target = s.camera.position + Vec3(std::cos(angle) * 22, 3, std::sin(angle) * 22);
        shark.target.y = std::max(shark.target.y, s.floorAt(shark.target) + 5);
    }
    Vec3 direction = safeNormalize(shark.target - shark.position);
    float bottom = s.floorAt(shark.position) + 4;
    if (shark.position.y < bottom) direction.y += bottom - shark.position.y;
    shark.velocity = mix(shark.velocity, safeNormalize(direction) * 3.0f, 1 - std::exp(-.45f * dt));
    shark.position += shark.velocity * dt;
    shark.yaw = std::atan2(-shark.velocity.z, shark.velocity.x);
    shark.pitch = std::atan2(shark.velocity.y, std::sqrt(shark.velocity.x * shark.velocity.x + shark.velocity.z * shark.velocity.z));
    shark.tailAngle = std::sin(shark.age * 2.5f) * .25f;
}

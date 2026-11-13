#pragma once
#include "Update.hpp"
#include <cstdint>
#include <string>

struct RenderStats {
    int draws=0,triangles=0,corals=0,fish=0,objectsCulled=0,chunksDrawn=0;
    int gpuChunks=0;
};
struct PhotoView {
    std::uint64_t textureId=0;
    int width=0,height=0;
    std::string credit,error;
};
struct Ui {
    bool showHelp_=false;
    void fieldGuide(Scene& scene,const std::array<PhotoView,SpeciesCount>& photos);
public:
    InputFrame build(Scene& scene,const RenderStats& stats,const std::array<PhotoView,SpeciesCount>& photos,
                     bool captured,float fps,float frameMs);
};

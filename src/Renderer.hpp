#pragma once
#include "Graphics.hpp"
#include "Scene.hpp"
#include "Interface.hpp"
#include <memory>
struct GLFWwindow;

class Renderer {
    struct Node {
        Mat4 local=Mat4(1.0f);Mesh* mesh=nullptr;Material* material=nullptr;
        Vec4 tint{1,1,1,1};std::vector<Node> children;
    };
    Shader shader_,background_,bubbleShader_,pickShader_;
    std::array<std::unique_ptr<Mesh>,CoralVariants> coral_;
    Mesh rock_,kelp_,sphere_,tail_,fin_,ring_,streamer_,sharkBody_,sharkTail_;
    Material sandMat_,coralMat_,rockMat_,fishMat_,leafMat_,plainMat_;
    Texture caustics_;
    std::array<Texture,SpeciesCount> photoTextures_;
    std::array<PhotoView,SpeciesCount> photoViews_;
    std::array<Node,SpeciesCount> fishNodes_;
    std::map<ChunkKey,std::unique_ptr<Mesh>> terrain_;
    std::uint64_t epoch_=~std::uint64_t(0);
    GLuint emptyVao_=0,bubbleVao_=0,bubbleVbo_=0;
    GLuint pickFbo_=0,pickTexture_=0,pickDepth_=0;
    int width_=0,height_=0,pickWidth_=0,pickHeight_=0;
    bool uiInitialized_=false;
    RenderStats stats_;
    void syncChunks(const Scene& scene);
    void prepareShader(Shader& shader,const Scene& scene,bool picking);
    void applyMaterial(Shader& shader,const Material& material);
    void draw(Shader& shader,Mesh& mesh,const Material& material,const Mat4& model,Vec4 tint,
              int fishPattern=-1,int variant=0,float emission=0,int pickId=0,float sway=0);
    void batch(Shader& shader,Mesh& mesh,const Material& material,const std::vector<Instance>& instances,float sway=0);
    void drawNode(Shader& shader,const Node& node,const Mat4& parent,int species,int variant,float emission,int id);
    void drawGeometry(const Scene& scene,bool picking);
    void drawMegalodon(Shader& shader, const Scene& scene, bool picking);
    bool ensurePickBuffer();
    void shutdownUI();
public:
    Renderer(const std::filesystem::path& shaders,const std::filesystem::path& assets);
    ~Renderer();
    Renderer(const Renderer&)=delete;Renderer& operator=(const Renderer&)=delete;
    void render(const Scene& scene,int width,int height);
    int pick(const Scene& scene,int framebufferX,int framebufferY);
    const RenderStats& stats() const {return stats_;}
    const std::array<PhotoView,SpeciesCount>& photos() const {return photoViews_;}
    bool screenshot(const std::filesystem::path& path) const;
    static bool checkErrors();
    bool valid() const;
    bool initializeUI(GLFWwindow* window,float scale=1);
    void beginUI();
    void endUI();
    bool wantsMouse() const;
};

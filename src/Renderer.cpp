#include "Renderer.hpp"
#include "Geometry.hpp"
#include <fstream>
#include <sstream>

#include <iostream>

static MeshData streamerMesh() {
    MeshData m; m.twoSided = true;
    for(int i=0;i<=12;++i) {
        float t=float(i)/12,w=.055f*(1-t)+.005f;
        Vec3 p{-.12f-.45f*t+.10f*std::sin(t*Pi),.3f+t*.9f,0};
        m.vertices.push_back({p+Vec3(w,0,0),{0,0,1},{0,t}});
        m.vertices.push_back({p-Vec3(w,0,0),{0,0,1},{1,t}});
        if(i<12){unsigned a=unsigned(i*2);m.indices.insert(m.indices.end(),{a,a+1,a+2,a+1,a+3,a+2});}
    }
    return m;
}
Renderer::Renderer(const std::filesystem::path& p,const std::filesystem::path& assets)
    :shader_(p/"reef.vert",p/"reef.frag"),background_(p/"background.vert",p/"background.frag"),
    bubbleShader_(p/"bubbles.vert",p/"bubbles.frag"),pickShader_(p/"reef.vert",p/"pick.frag"),
    rock_(makeRock()),kelp_(makeKelp()),sphere_(makeSphere(18,12)),tail_(makeTail()),fin_(makeFin()),ring_(makeRing()),streamer_(streamerMesh()),sharkBody_(makeSharkBody()),sharkTail_(makeSharkTail()) {
    for(int i=0;i<CoralVariants;++i)coral_[std::size_t(i)]=std::make_unique<Mesh>(makeCoralVariant(i));
    makeMaterial(sandMat_,0);sandMat_.uvScale=1;makeMaterial(coralMat_,1);makeMaterial(rockMat_,2);
    makeMaterial(fishMat_,3);makeMaterial(leafMat_,4);makeMaterial(plainMat_,5);makeCaustics(caustics_);
    glGenVertexArrays(1,&emptyVao_);glGenVertexArrays(1,&bubbleVao_);glGenBuffers(1,&bubbleVbo_);
    glBindVertexArray(bubbleVao_);glBindBuffer(GL_ARRAY_BUFFER,bubbleVbo_);
    glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,sizeof(Vec4),nullptr);glEnableVertexAttribArray(0);glBindVertexArray(0);
    for(int i=0;i<SpeciesCount;++i) {
        const auto& species=SpeciesList[std::size_t(i)];Node body,pivot,tail,dorsal,bottom,side,eye,eye2,mouth;
        body.mesh=&sphere_;body.material=&fishMat_;body.local=scale(species.bodyShape);
        pivot.local=translate({-.43f,0,0});tail.mesh=&tail_;tail.material=&fishMat_;pivot.children.push_back(tail);
        dorsal.mesh=&fin_;dorsal.material=&fishMat_;dorsal.local=translate({-.06f,species.bodyShape.y*.68f,0})*scale({1,1.2f,1});
        bottom=dorsal;bottom.local=translate({0,-species.bodyShape.y*.65f,0})*rotateX(Pi)*scale(Vec3(.7f));
        side=dorsal;side.local=translate({.05f,-.025f,species.bodyShape.z*.75f})*rotateX(radians(65))*scale(Vec3(.55f));
        eye.mesh=&sphere_;eye.material=&plainMat_;eye.tint={.008f,.017f,.021f,1};
        eye.local=translate({species.bodyShape.x*.63f,.065f,species.bodyShape.z*.80f})*scale(Vec3(.032f));
        eye2=eye;eye2.local=translate({species.bodyShape.x*.63f,.065f,-species.bodyShape.z*.80f})*scale(Vec3(.032f));
        fishNodes_[std::size_t(i)].children={body,pivot,dorsal,bottom,side,eye,eye2};
        if(i==2||i==4||i==5) {
            mouth.mesh=&sphere_;mouth.material=&fishMat_;mouth.local=translate({species.bodyShape.x*.93f,-.055f,0})*scale({.16f,.06f,.055f});
            fishNodes_[std::size_t(i)].children.push_back(mouth);
        }
        if(i==5) {Node n;n.mesh=&streamer_;n.material=&fishMat_;fishNodes_[std::size_t(i)].children.push_back(n);}
        if(i==3) {Node n;n.mesh=&sphere_;n.material=&fishMat_;n.local=translate({.28f,.16f,0})*scale({.23f,.25f,.18f});fishNodes_[std::size_t(i)].children.push_back(n);}
        auto& view=photoViews_[std::size_t(i)];auto path=assets/"fish"/species.photoFile;
        if(photoTextures_[std::size_t(i)].loadBMP(path,view.error)) {
            const auto& texture=photoTextures_[std::size_t(i)];view.textureId=texture.id;view.width=texture.width;view.height=texture.height;
        }
        else std::cerr << "WARNING: fish photo " << path << ": " << view.error << '\n';
        std::ifstream file(path.string()+".credit.txt");
        if(file){std::ostringstream text;text<<file.rdbuf();view.credit=text.str();}
    }
    glEnable(GL_DEPTH_TEST);glEnable(GL_MULTISAMPLE);
    glFrontFace(GL_CCW); glCullFace(GL_BACK);
}
Renderer::~Renderer() {
    shutdownUI();
    glDeleteFramebuffers(1,&pickFbo_);glDeleteTextures(1,&pickTexture_);glDeleteRenderbuffers(1,&pickDepth_);
    glDeleteBuffers(1,&bubbleVbo_);glDeleteVertexArrays(1,&bubbleVao_);glDeleteVertexArrays(1,&emptyVao_);
}
void Renderer::syncChunks(const Scene& s) {
    if(epoch_!=s.generation){terrain_.clear();epoch_=s.generation;}
    for(auto it=terrain_.begin();it!=terrain_.end();) {if(!s.chunks.count(it->first))it=terrain_.erase(it);else ++it;}
    for(const auto& item:s.chunks)if(!terrain_.count(item.first))terrain_.emplace(item.first,std::make_unique<Mesh>(item.second.terrain));
}
void Renderer::prepareShader(Shader& shader,const Scene& s,bool picking) {
    shader.use();
    shader.set("uView",s.camera.view());shader.set("uProjection",perspective(s.camera.fov,float(width_)/float(height_),.08f,s.settings.visibility+25));
    shader.set("uEye",s.camera.position);shader.set("uTime",float(std::fmod(s.time,48000.0)));
    shader.set("uVisibility",s.settings.visibility);
    if(picking)return;
    shader.set("uLightDirection",s.light.direction);shader.set("uAmbient",s.light.ambient);shader.set("uLightColor",s.light.color);
    shader.set("uFogColor",s.light.fogColor);shader.set("uDaylight",s.light.daylight);shader.set("uFogStart",s.settings.fogStart);
    shader.set("uWorldOffset",Vec2{float((s.origin.first%2500)*24),float((s.origin.second%2500)*24)});
    shader.set("uLighting",int(s.settings.lighting));shader.set("uTexturing",int(s.settings.textures));shader.set("uNormalMapping",int(s.settings.normalMaps));
    shader.set("uShowCaustics",int(s.settings.caustics));shader.set("uFog",int(s.settings.fog));
    shader.set("uDiffuse",0);shader.set("uNormalMap",1);shader.set("uCaustics",2);caustics_.bind(2);
}
void Renderer::applyMaterial(Shader& shader,const Material& m) {
    m.diffuse.bind(0);m.normal.bind(1);shader.set("uUVScale",m.uvScale);shader.set("uShininess",m.shininess);shader.set("uSpecular",m.specular);
}
void Renderer::draw(Shader& shader,Mesh& mesh,const Material& mat,const Mat4& model,Vec4 tint,int pattern,int variant,float emission,int id,float sway) {
    applyMaterial(shader,mat);shader.set("uInstanced",0);shader.set("uModel",model);shader.set("uTint",tint);
    shader.set("uFishPattern",pattern);shader.set("uFishVariant",variant);shader.set("uEmission",emission);shader.set("uPickId",id);shader.set("uSway",sway);
    if (mesh.twoSided) glDisable(GL_CULL_FACE); else glEnable(GL_CULL_FACE);
    glBindVertexArray(mesh.vao); glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, nullptr);++stats_.draws;stats_.triangles+=mesh.triangles();
}
void Renderer::batch(Shader& shader,Mesh& mesh,const Material& mat,const std::vector<Instance>& data,float sway) {
    if(data.empty())return;
    mesh.setInstances(data);applyMaterial(shader,mat);shader.set("uInstanced",1);shader.set("uSway",sway);
    shader.set("uFishPattern",-1);shader.set("uFishVariant",0);shader.set("uEmission",0.0f);shader.set("uPickId",0);
    if (mesh.twoSided) glDisable(GL_CULL_FACE); else glEnable(GL_CULL_FACE);
    glBindVertexArray(mesh.vao); glDrawElementsInstanced(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, nullptr, mesh.instanceCount);++stats_.draws;stats_.triangles+=mesh.triangles()*int(data.size());
}
void Renderer::drawNode(Shader& shader,const Node& node,const Mat4& parent,int species,int variant,float emission,int id) {
    Mat4 world=parent*node.local;
    if(node.mesh&&node.material)draw(shader,*node.mesh,*node.material,world,node.tint,node.material==&fishMat_?species:-1,variant,emission,id);
    for(const auto& child:node.children)drawNode(shader,child,world,species,variant,emission,id);
}
void Renderer::drawGeometry(const Scene& s,bool picking) {
    Shader& shader=picking?pickShader_:shader_;prepareShader(shader,s,picking);
    const float range=s.settings.visibility;
    Frustum frustum(perspective(s.camera.fov,float(width_)/float(height_),.08f,range+25)*s.camera.view());
    auto visible=[&](Vec3 p,float radius){return sphereInRange(p,radius,s.camera.position,range)&&frustum.containsSphere(p,radius);};
    std::array<std::vector<Instance>,CoralVariants> corals;std::vector<Instance> rocks,kelp;
    for(const auto& item:s.chunks) {
        Vec3 offset=chunkOffset(item.first,s.origin);
        if(!visible(offset,23.0f)){stats_.objectsCulled+=int(item.second.objects.size())+1;continue;}
        auto mesh=terrain_.find(item.first);if(mesh!=terrain_.end()){draw(shader,*mesh->second,sandMat_,translate(offset),{1,1,1,1});++stats_.chunksDrawn;}
        for(const auto& o:item.second.objects) {
            if(!visible(offset+o.boundCenter,o.radius)){++stats_.objectsCulled;continue;}
            Instance inst{translate(offset)*o.transform.matrix(),o.tint};
            if(o.kind==ObjectKind::Coral){corals[std::size_t(o.variant)].push_back(inst);++stats_.corals;}
            else if(o.kind==ObjectKind::Rock)rocks.push_back(inst);else kelp.push_back(inst);
        }
    }
    batch(shader,rock_,rockMat_,rocks);
    for(int i=0;i<CoralVariants;++i)batch(shader,*coral_[std::size_t(i)],coralMat_,corals[std::size_t(i)]);
    batch(shader,kelp_,leafMat_,kelp,1);
    Vec3 originOffset=chunkOffset({0,0},s.origin),featured=originOffset+s.featured.position;
    if(visible(featured+Vec3(0,1.4f*s.featured.size.y,0),2.8f*std::max({s.featured.size.x,s.featured.size.y,s.featured.size.z}))) {
        Vec3 base=featured;base.y=terrainHeight(s.featured.position.x,s.featured.position.z,s.settings.seed)+.03f;
        draw(shader,rock_,rockMat_,translate(base)*scale({1.05f,.25f,1.05f}),{.86f,.93f,.84f,1});
        draw(shader,*coral_[0],coralMat_,translate(originOffset)*s.featured.matrix(),{1,.37f,.44f,1});++stats_.corals;
        if(s.settings.ui&&!picking)draw(shader,ring_,plainMat_,translate(base),{.90f,.77f,.37f,1},-1,0,.24f);
    }
    for(int i=0;i<int(s.fish.size());++i) {
        const Fish& f=s.fish[std::size_t(i)];
        if(!visible(f.position,f.scale*1.5f)){++stats_.objectsCulled;continue;}
        Node& root=fishNodes_[std::size_t(f.species)];
        root.local=translate(f.position)*rotateY(f.yaw)*rotateZ(f.pitch)*rotateX(f.roll)*scale(Vec3(f.scale));
        for(auto& node:root.children)if(node.material==&fishMat_)node.tint=f.tint;
        root.children[1].children[0].tint=f.species==1?Vec4{1,.85f,.06f,1}:f.tint;
        root.children[1].local=translate({-.43f,0,0})*rotateY(f.tailAngle);
        root.children[4].local=translate({.05f,-.025f,SpeciesList[std::size_t(f.species)].bodyShape.z*.75f})*
            rotateX(radians(65)+f.finAngle)*scale(Vec3(.55f));
        drawNode(shader,root,Mat4(1.0f),f.species,f.variant,i==s.selectedFish?.22f:0,i+1);++stats_.fish;
    }
    if (s.megalodon.active && visible(s.megalodon.position, 9.5f)) drawMegalodon(shader, s, picking);
}
void Renderer::render(const Scene& s,int width,int height) {
    if(width<=0||height<=0)return;
    width_=width;height_=height;syncChunks(s);stats_={};stats_.gpuChunks=int(terrain_.size());
    glBindFramebuffer(GL_FRAMEBUFFER,0);glViewport(0,0,width,height);glDepthMask(GL_TRUE);
    glClearColor(s.light.fogColor.x,s.light.fogColor.y,s.light.fogColor.z,1);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);glDisable(GL_DEPTH_TEST);glDisable(GL_BLEND);glDisable(GL_CULL_FACE);
    background_.use();background_.set("uForward",s.camera.forward());background_.set("uRight",s.camera.right());background_.set("uUp",s.camera.up());
    background_.set("uAspect",float(width)/float(height));background_.set("uTanHalfFov",std::tan(s.camera.fov*.5f));
    background_.set("uLightDirection",s.light.direction);background_.set("uFogColor",s.light.fogColor);background_.set("uLightColor",s.light.color);background_.set("uDaylight",s.light.daylight);
    glBindVertexArray(emptyVao_);glDrawArrays(GL_TRIANGLES,0,3);++stats_.draws;
    glEnable(GL_DEPTH_TEST);glDepthFunc(GL_LESS);glPolygonMode(GL_FRONT_AND_BACK,s.settings.wireframe?GL_LINE:GL_FILL);
    drawGeometry(s,false);glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
    if(s.settings.bubbles) {
        std::vector<Vec4> bubbles;for(auto b:s.bubbles)if(sphereInRange({b.x,b.y,b.z},b.w,s.camera.position,s.settings.visibility))bubbles.push_back(b);
        glBindVertexArray(bubbleVao_);glBindBuffer(GL_ARRAY_BUFFER,bubbleVbo_);
        glBufferData(GL_ARRAY_BUFFER,static_cast<GLsizeiptr>(bubbles.size()*sizeof(Vec4)),bubbles.data(),GL_STREAM_DRAW);
        bubbleShader_.use();bubbleShader_.set("uView",s.camera.view());bubbleShader_.set("uProjection",perspective(s.camera.fov,float(width)/float(height),.08f,s.settings.visibility+25));
        bubbleShader_.set("uHeight",float(height));bubbleShader_.set("uFogStart",s.settings.fogStart);bubbleShader_.set("uVisibility",s.settings.visibility);
        bubbleShader_.set("uFog",int(s.settings.fog));bubbleShader_.set("uDaylight",s.light.daylight);
        glEnable(GL_PROGRAM_POINT_SIZE);glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE);glDepthMask(GL_FALSE);
        glDrawArrays(GL_POINTS,0,static_cast<GLsizei>(bubbles.size()));++stats_.draws;glDepthMask(GL_TRUE);glDisable(GL_BLEND);
    }
    glBindVertexArray(0);
}
bool Renderer::ensurePickBuffer() {
    if(pickFbo_&&pickWidth_==width_&&pickHeight_==height_)return true;
    if(!pickFbo_){glGenFramebuffers(1,&pickFbo_);glGenTextures(1,&pickTexture_);glGenRenderbuffers(1,&pickDepth_);}
    glBindTexture(GL_TEXTURE_2D,pickTexture_);
    glTexImage2D(GL_TEXTURE_2D,0,GL_R32UI,width_,height_,0,GL_RED_INTEGER,GL_UNSIGNED_INT,nullptr);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glBindRenderbuffer(GL_RENDERBUFFER,pickDepth_);glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH_COMPONENT24,width_,height_);
    glBindFramebuffer(GL_FRAMEBUFFER,pickFbo_);glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,pickTexture_,0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,pickDepth_);glDrawBuffer(GL_COLOR_ATTACHMENT0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "ERROR: fish-picking framebuffer is incomplete\n";
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return false;
    }
    pickWidth_=width_;pickHeight_=height_;
    return true;
}
int Renderer::pick(const Scene& s,int x,int y) {
    if(width_<=0||height_<=0||x<0||y<0||x>=width_||y>=height_)return -1;
    RenderStats saved=stats_;if (!ensurePickBuffer()) return -1;glBindFramebuffer(GL_FRAMEBUFFER,pickFbo_);glViewport(0,0,width_,height_);
    glDisable(GL_BLEND);glDisable(GL_CULL_FACE);glEnable(GL_DEPTH_TEST);glDepthMask(GL_TRUE);glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
    const GLuint zero[4]={0,0,0,0};glClearBufferuiv(GL_COLOR,0,zero);glClear(GL_DEPTH_BUFFER_BIT);drawGeometry(s,true);
    GLuint id=0;glReadBuffer(GL_COLOR_ATTACHMENT0);glReadPixels(x,y,1,1,GL_RED_INTEGER,GL_UNSIGNED_INT,&id);
    glBindFramebuffer(GL_FRAMEBUFFER,0);glReadBuffer(GL_BACK);stats_=saved;
    return id > 0 && id <= s.fish.size() + (s.megalodon.active ? 1u : 0u) ? int(id) - 1 : -1;
}
bool Renderer::screenshot(const std::filesystem::path& path) const {return saveScreenshot(path,width_,height_);}
bool Renderer::valid() const {
    return shader_.valid() && background_.valid() && bubbleShader_.valid() && pickShader_.valid();
}
bool Renderer::checkErrors() {
    bool ok = true;
    for (GLenum error = glGetError(); error != GL_NO_ERROR; error = glGetError()) {
        std::cerr << "ERROR: OpenGL code " << error << '\n';
        ok = false;
    }
    return ok;
}

void Renderer::drawMegalodon(Shader& shader, const Scene& s, bool) {
    const Megalodon& shark = s.megalodon;
    const Mat4 model = translate(shark.position) * rotateY(shark.yaw) * rotateZ(shark.pitch) * scale(Vec3(1.65f));
    const int id = int(s.fish.size()) + 1;
    const Vec4 grey(.27f, .35f, .39f, 1);
    draw(shader, sharkBody_, fishMat_, model, grey, -2, 0, 0, id);
    draw(shader, sharkTail_, fishMat_, model * translate({-3.45f, 0, 0}) * rotateY(shark.tailAngle), grey, -1, 0, 0, id);
    draw(shader, fin_, fishMat_, model * translate({-.4f, .50f, 0}) * scale({3.7f, 5.5f, 1}), grey, -1, 0, 0, id);
    for (int side : {-1, 1}) {
        draw(shader, fin_, fishMat_, model * translate({.4f, -.2f, float(side) * .48f}) * rotateX(float(side) * radians(100)) * scale({4.8f, 5.0f, 1}), grey, -1, 0, 0, id);
        draw(shader, sphere_, plainMat_, model * translate({2.5f, .12f, float(side) * .34f}) * scale(Vec3(.09f)), {.006f, .01f, .012f, 1}, -1, 0, 0, id);
    }
}

#include "Scene.hpp"
#include "Geometry.hpp"
#include <array>

namespace reef {
Scene::Scene(const std::filesystem::path& p)
    :shader(p/"reef.vert",p/"reef.frag"),background(p/"background.vert",p/"background.frag"),
     bubbleShader(p/"bubbles.vert",p/"bubbles.frag"),sand(makeSeabed()),branch(makeBranchCoral()),
     tube(makeTubeCoral()),plate(makePlateCoral()),rock(makeRock()),kelp(makeKelp()),
     sphere(makeSphere(16,10)),tail(makeTail()),fin(makeFin()),ring(makeRing()) {
    makeMaterial(sandMaterial,0);makeMaterial(coralMaterial,1);makeMaterial(rockMaterial,2);
    makeMaterial(fishMaterial,3);makeMaterial(leafMaterial,4);makeMaterial(plainMaterial,5);
    makeCaustics(causticTexture);featured.reset();
    glGenVertexArrays(1,&emptyVAO);glGenVertexArrays(1,&bubbleVAO);glGenBuffers(1,&bubbleVBO);
    glBindVertexArray(bubbleVAO);glBindBuffer(GL_ARRAY_BUFFER,bubbleVBO);
    glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,sizeof(Vec4),nullptr);glEnableVertexAttribArray(0);
    Random rng(3478);
    for(int i=0;i<150;++i) bubbleOrigins.push_back({rng.range(-17,17),rng.range(0,13),rng.range(-24,9),rng.range(.018f,.055f)});
    // The tail is a child of a pivot, itself a child of the swimming fish root.
    Node body;body.mesh=&sphere;body.material=&fishMaterial;body.local=scale({.50f,.235f,.125f});
    Node pivot;pivot.local=translate({-.43f,0,0});
    Node tailNode;tailNode.mesh=&tail;tailNode.material=&fishMaterial;pivot.children.push_back(tailNode);
    Node dorsal;dorsal.mesh=&fin;dorsal.material=&fishMaterial;dorsal.local=translate({0,.16f,0});
    Node bottom=dorsal;bottom.local=translate({0,-.15f,0})*rotateX(Pi)*scale({.7f,.7f,.7f});
    Node side=dorsal;side.local=translate({.05f,-.025f,.085f})*rotateX(radians(65))*scale({.55f,.55f,.55f});
    Node eye;eye.mesh=&sphere;eye.material=&plainMaterial;eye.tint={.015f,.025f,.025f,1};
    eye.local=translate({.29f,.065f,.105f})*scale(Vec3(.039f));
    Node eye2=eye;eye2.local=translate({.29f,.065f,-.105f})*scale(Vec3(.039f));
    fish.children={body,pivot,dorsal,bottom,side,eye,eye2};
}
Scene::~Scene() { glDeleteBuffers(1,&bubbleVBO);glDeleteVertexArrays(1,&bubbleVAO);glDeleteVertexArrays(1,&emptyVAO); }
void Scene::regenerate(const Settings& settings) {
    const int targets[3]={40,76,128};const float spacing[3]={2.7f,2.2f,1.75f};
    auto points=scatter(settings.seed,targets[settings.quality],spacing[settings.quality]);
    coralCount=static_cast<int>(points.size());fishCount=settings.quality==0?14:24;
    Random rng(settings.seed+71);
    std::array<std::vector<Instance>,3> corals;
    std::vector<Instance> rocks,leaves;
    const Vec4 palette[]={ {1.0f,.31f,.37f,1},{.81f,.28f,.70f,1},{.53f,.40f,.97f,1},
                          {.27f,.88f,.74f,1},{1.0f,.72f,.25f,1},{1.0f,.45f,.22f,1} };
    for(Vec2 point:points) {
        int type=rng.integer(0,2);float s=rng.range(.66f,1.23f);
        float y=seabedHeight(point.x,point.y),angle=rng.range(0,2*Pi);
        Vec4 color=palette[rng.integer(0,5)];
        Vec3 size(s*rng.range(.9f,1.12f),s*rng.range(.85f,1.3f),s);
        corals[type].push_back({compose({point.x,y+.22f,point.y},{0,angle,0},size),color});
        rocks.push_back({compose({point.x,y+.03f,point.y},{0,angle,0},{s*.83f,s*.29f,s*.8f}),{.88f,.95f,.89f,1}});
    }
    // Extra isolated boulders and curved leaf clumps fill the back of the scene.
    for(int i=0;i<30;++i) {
        float x=rng.range(-18,18),z=rng.range(-26,5),s=rng.range(.3f,1.4f);
        if(reservedSpace(x,z)) continue;
        rocks.push_back({compose({x,seabedHeight(x,z),z},{0,rng.range(0,2*Pi),0},{s,s*.57f,s*.85f}),{.72f,.82f,.76f,1}});
        if(i<24) leaves.push_back({compose({x+.6f,seabedHeight(x+.6f,z),z},{0,rng.range(0,2*Pi),0},Vec3(rng.range(.7f,1.2f))),{.48f,.85f,.62f,1}});
    }
    branch.setInstances(corals[0]);tube.setInstances(corals[1]);plate.setInstances(corals[2]);
    rock.setInstances(rocks);kelp.setInstances(leaves);
}
void Scene::material(const Material& m) {
    m.diffuse.bind(0);m.normal.bind(1);
    shader.set("uUVScale",m.uvScale);shader.set("uShininess",m.shininess);shader.set("uSpecular",m.specular);
}
void Scene::draw(Mesh& mesh,Material& m,const Mat4& model,Vec4 tint,float sway,float emission) {
    material(m);shader.set("uInstanced",0);shader.set("uModel",model);shader.set("uTint",tint);
    shader.set("uSway",sway);shader.set("uEmission",emission);mesh.draw();++drawCalls;triangleCount+=mesh.triangles();
}
void Scene::drawInstances(Mesh& mesh,Material& m,float sway) {
    if(mesh.instances()==0)return;
    material(m);shader.set("uInstanced",1);shader.set("uSway",sway);shader.set("uEmission",0.0f);
    mesh.drawInstances();++drawCalls;triangleCount+=mesh.triangles()*mesh.instances();
}
void Scene::drawNode(const Node& node,const Mat4& parent) {
    // Hierarchical modeling: world = parentWorld * local.
    Mat4 world=parent*node.local;
    if(node.mesh && node.material)draw(*node.mesh,*node.material,world,node.tint);
    for(const Node& child:node.children)drawNode(child,world);
}
void Scene::render(const Camera& camera,const Settings& s,float time,int width,int height) {
    drawCalls=0;triangleCount=0;
    float aspect=float(width)/height;
    Mat4 view=camera.view(),projection=perspective(camera.fov,aspect,.08f,100.0f);
    Vec3 light=normalize(Vec3(.38f+std::sin(time*.13f)*.22f,1.0f,.35f+std::cos(time*.11f)*.2f));
    glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);glDisable(GL_DEPTH_TEST);glDisable(GL_BLEND);
    background.use();background.set("uForward",camera.forward());background.set("uRight",camera.right());background.set("uUp",camera.up());
    background.set("uLightDirection",light);background.set("uAspect",aspect);background.set("uTanHalfFov",std::tan(camera.fov*.5f));
    glBindVertexArray(emptyVAO);glDrawArrays(GL_TRIANGLES,0,3);++drawCalls;
    glEnable(GL_DEPTH_TEST);glDepthMask(GL_TRUE);glPolygonMode(GL_FRONT_AND_BACK,s.wireframe?GL_LINE:GL_FILL);
    // Deliberately two-sided geometry for kelp and fish fins. Depth testing remains enabled.
    glDisable(GL_CULL_FACE);
    shader.use();shader.set("uView",view);shader.set("uProjection",projection);shader.set("uEye",camera.position);
    shader.set("uLightDirection",light);shader.set("uTime",time);
    shader.set("uLighting",int(s.lighting));shader.set("uTexturing",int(s.textures));shader.set("uNormalMapping",int(s.normalMaps));
    shader.set("uShowCaustics",int(s.caustics));shader.set("uFog",int(s.fog));
    shader.set("uDiffuse",0);shader.set("uNormalMap",1);shader.set("uCaustics",2);causticTexture.bind(2);
    draw(sand,sandMaterial,Mat4::identity(),{1,1,1,1});
    drawInstances(rock,rockMaterial);drawInstances(branch,coralMaterial);drawInstances(tube,coralMaterial);drawInstances(plate,coralMaterial);
    drawInstances(kelp,leafMaterial,1.0f);
    Vec3 base=featured.position;base.y=seabedHeight(base.x,base.z)+.02f;
    draw(rock,rockMaterial,translate(base)*scale({1.05f,.30f,1.05f}),{.94f,.97f,.93f,1});
    draw(branch,coralMaterial,featured.matrix(),{1.0f,.37f,.44f,1});
    if(s.ui) draw(ring,plainMaterial,translate({base.x,seabedHeight(base.x,base.z)+.08f,base.z}),{.90f,.77f,.37f,1},0,.30f);
    for(int i=0;i<fishCount;++i) {
        bool group=(i%2)==0;float phase=float(i)*.61f,timeScale=group?.30f:-.24f;
        float a=time*timeScale+phase,rx=group?6.5f:4.8f,rz=group?4.0f:3.3f;
        Vec3 center=group?Vec3(-3.0f,2.4f,-8.0f):Vec3(4.2f,3.5f,-9.5f);
        Vec3 p=center+Vec3(std::cos(a)*rx,.45f*std::sin(a*1.7f+phase),std::sin(a)*rz);
        Vec3 velocity(-std::sin(a)*rx*timeScale,0,std::cos(a)*rz*timeScale);
        float yaw=std::atan2(-velocity.z,velocity.x),size=.75f+float(i%4)*.09f;
        fish.local=translate(p)*rotateY(yaw)*scale(Vec3(size));
        Vec4 tint=group?Vec4{1.0f,.78f,.25f,1}:Vec4{.40f,.86f,1.0f,1};
        fish.children[0].tint=tint;fish.children[1].children[0].tint=tint;
        fish.children[2].tint=fish.children[3].tint=fish.children[4].tint=tint;
        fish.children[1].local=translate({-.43f,0,0})*rotateY(std::sin(time*8+phase)*.48f);
        drawNode(fish,Mat4::identity());
    }
    glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
    if(s.bubbles) {
        std::vector<Vec4> positions;positions.reserve(bubbleOrigins.size());
        for(Vec4 b:bubbleOrigins) {
            float y=std::fmod(b.y+time*(.20f+b.w*3),13.0f);
            positions.push_back({b.x+.20f*std::sin(time*.5f+b.z),y,b.z,b.w});
        }
        glBindVertexArray(bubbleVAO);glBindBuffer(GL_ARRAY_BUFFER,bubbleVBO);
        glBufferData(GL_ARRAY_BUFFER,static_cast<GLsizeiptr>(positions.size()*sizeof(Vec4)),positions.data(),GL_STREAM_DRAW);
        bubbleShader.use();bubbleShader.set("uView",view);bubbleShader.set("uProjection",projection);bubbleShader.set("uHeight",float(height));
        glEnable(GL_PROGRAM_POINT_SIZE);glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE);glDepthMask(GL_FALSE);
        glDrawArrays(GL_POINTS,0,static_cast<GLsizei>(positions.size()));++drawCalls;
        glDepthMask(GL_TRUE);glDisable(GL_BLEND);
    }
    glBindVertexArray(0);
}
} // namespace reef

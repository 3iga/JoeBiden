#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "Scene.hpp"
#include "UI.hpp"
#include <array>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>

using namespace reef;
namespace {
struct Options {
    int width=1280,height=720,frames=0,captureEvery=1;
    float fixedDt=0;
    std::string capture,captureDir;
    bool checkGL=false;
    Settings settings;
};
std::string number(float v,int precision=1) { std::ostringstream s;s<<std::fixed<<std::setprecision(precision)<<v;return s.str(); }
std::string vectorText(Vec3 v) { return number(v.x)+" / "+number(v.y)+" / "+number(v.z); }
void help() {
    std::cout<<"Luminous Reef -- C++17 / modern OpenGL\n"
      "Controls: WASD move, SPACE/CTRL up/down, SHIFT faster, mouse look, scroll zoom.\n"
      "TAB release cursor; click fish to scan it. G tour, HOME reset camera, F2 focus coral.\n"
      "B lighting, C caustics, T textures, N normal maps, F fog, M wireframe, V vsync.\n"
      "1 move / 2 rotate / 3 scale featured coral; J/L X, K/I Y, U/O Z; BACKSPACE reset.\n"
      "H hide UI, F12 screenshot, ESC exit.\n\n"
      "Options: --seed N --quality low|medium|high --width N --height N\n"
      "         --tour --no-ui --no-vsync --no-caustics --no-fog --unlit\n"
      "         --no-textures --no-normals --wireframe --no-bubbles\n"
      "         --frames N --fixed-dt SECONDS --capture FILE.bmp\n"
      "         --capture-dir DIR --capture-every N --check-gl\n";
}
Options parse(int argc,char** argv) {
    Options o;
    auto value=[&](int& i)->std::string { if(++i>=argc)throw std::runtime_error("Missing command-line value");return argv[i]; };
    for(int i=1;i<argc;++i) {
        std::string a=argv[i];
        if(a=="--width")o.width=std::stoi(value(i));else if(a=="--height")o.height=std::stoi(value(i));
        else if(a=="--seed")o.settings.seed=static_cast<std::uint32_t>(std::stoul(value(i)));
        else if(a=="--frames")o.frames=std::stoi(value(i));else if(a=="--fixed-dt")o.fixedDt=std::stof(value(i));
        else if(a=="--capture")o.capture=value(i);else if(a=="--capture-dir")o.captureDir=value(i);
        else if(a=="--capture-every")o.captureEvery=std::stoi(value(i));
        else if(a=="--quality") {std::string q=value(i);if(q=="low")o.settings.quality=0;else if(q=="medium")o.settings.quality=1;else if(q=="high")o.settings.quality=2;else throw std::runtime_error("Quality must be low, medium or high");}
        else if(a=="--tour")o.settings.tour=true;else if(a=="--no-ui")o.settings.ui=false;
        else if(a=="--no-vsync")o.settings.vsync=false;else if(a=="--no-caustics")o.settings.caustics=false;
        else if(a=="--no-fog")o.settings.fog=false;else if(a=="--unlit")o.settings.lighting=false;
        else if(a=="--no-textures")o.settings.textures=false;else if(a=="--no-normals")o.settings.normalMaps=false;
        else if(a=="--wireframe")o.settings.wireframe=true;else if(a=="--no-bubbles")o.settings.bubbles=false;
        else if(a=="--check-gl")o.checkGL=true;
        else throw std::runtime_error("Unknown option: "+a+". Use --help.");
    }
    if(o.width<320||o.height<240||o.width>7680||o.height>4320||o.frames<0||o.captureEvery<1||o.fixedDt<0||o.fixedDt>.25f)
        throw std::runtime_error("Invalid dimensions, frame count, capture interval, or fixed timestep");
    if(!o.capture.empty()&&o.frames==0)throw std::runtime_error("--capture requires --frames N; use F12 for interactive screenshots");
    return o;
}
struct Input {
    std::array<bool,GLFW_KEY_LAST+1> pressed{};
    bool click=false,captured=false,firstMouse=true;
    double lastX=0,lastY=0,scroll=0;
    void clear() { pressed.fill(false);click=false;scroll=0; }
};
void setCapture(GLFWwindow* window,Input& in,bool captured) {
    in.captured=captured;in.firstMouse=true;
    glfwSetInputMode(window,GLFW_CURSOR,captured?GLFW_CURSOR_DISABLED:GLFW_CURSOR_NORMAL);
    if(glfwRawMouseMotionSupported())glfwSetInputMode(window,GLFW_RAW_MOUSE_MOTION,captured?GLFW_TRUE:GLFW_FALSE);
}
struct Window {
    GLFWwindow* handle=nullptr;
    Window(int width,int height) {
        glfwSetErrorCallback([](int code,const char* text){std::cerr<<"GLFW "<<code<<": "<<text<<'\n';});
        if(!glfwInit())throw std::runtime_error("GLFW initialization failed. Check your graphics driver.");
        const int contexts[3][3]={{4,1,4},{3,3,4},{3,3,0}};
        for(const auto& version:contexts) {
            glfwDefaultWindowHints();glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,version[0]);glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,version[1]);
            glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);glfwWindowHint(GLFW_SAMPLES,version[2]);
#ifdef __APPLE__
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT,GLFW_TRUE);
#endif
            handle=glfwCreateWindow(width,height,"Luminous Reef",nullptr,nullptr);
            if(handle)break;
        }
        if(!handle) {glfwTerminate();throw std::runtime_error("An OpenGL 3.3+ CORE context is required. Update your GPU driver.");}
        glfwMakeContextCurrent(handle);
        if(!gladLoadGL(reinterpret_cast<GLADloadfunc>(glfwGetProcAddress))) {
            glfwDestroyWindow(handle);glfwTerminate();handle=nullptr;throw std::runtime_error("GLAD could not load OpenGL functions.");
        }
        glfwSetWindowSizeLimits(handle,1000,660,GLFW_DONT_CARE,GLFW_DONT_CARE);
        std::cout<<"OpenGL: "<<glGetString(GL_VERSION)<<"\nGPU: "<<glGetString(GL_RENDERER)<<'\n';
    }
    ~Window() { if(handle)glfwDestroyWindow(handle);glfwTerminate(); }
};
void focusCoral(Camera& camera,const Scene& scene,Settings& settings) {
    camera.position=scene.featured.position+Vec3(4.2f,2.8f,5.5f);
    camera.lookToward(scene.featured.position+Vec3(0,1.0f,0));
    camera.fov=radians(58);settings.tour=false;
}
void keyboard(GLFWwindow* w,Input& in,Camera& camera,Scene& scene,Settings& s,float dt) {
    auto press=[&](int key){return in.pressed[std::size_t(key)];};
    auto down=[&](int key){return glfwGetKey(w,key)==GLFW_PRESS;};
    if(press(GLFW_KEY_ESCAPE))glfwSetWindowShouldClose(w,GLFW_TRUE);
    if(press(GLFW_KEY_TAB))setCapture(w,in,!in.captured);
    if(press(GLFW_KEY_B))s.lighting=!s.lighting;
    if(press(GLFW_KEY_C))s.caustics=!s.caustics;
    if(press(GLFW_KEY_T))s.textures=!s.textures;
    if(press(GLFW_KEY_N))s.normalMaps=!s.normalMaps;
    if(press(GLFW_KEY_F))s.fog=!s.fog;
    if(press(GLFW_KEY_P))s.paused=!s.paused;
    if(press(GLFW_KEY_M))s.wireframe=!s.wireframe;
    if(press(GLFW_KEY_H))s.ui=!s.ui;
    if(press(GLFW_KEY_G))s.tour=!s.tour;
    if(press(GLFW_KEY_V)){s.vsync=!s.vsync;glfwSwapInterval(s.vsync?1:0);}
    if(press(GLFW_KEY_R)){++s.seed;scene.regenerate(s);}
    if(press(GLFW_KEY_Z)){s.quality=(s.quality+1)%3;scene.regenerate(s);}
    if(press(GLFW_KEY_HOME)){camera.reset();s.tour=false;in.firstMouse=true;}
    if(press(GLFW_KEY_F2)){focusCoral(camera,scene,s);in.firstMouse=true;}
    if(press(GLFW_KEY_BACKSPACE))scene.featured.reset();
    if(press(GLFW_KEY_1))s.transformMode=0;
    if(press(GLFW_KEY_2))s.transformMode=1;
    if(press(GLFW_KEY_3))s.transformMode=2;
    Vec3 axis(float(down(GLFW_KEY_L))-float(down(GLFW_KEY_J)),float(down(GLFW_KEY_I))-float(down(GLFW_KEY_K)),float(down(GLFW_KEY_O))-float(down(GLFW_KEY_U)));
    if(s.transformMode==0)scene.featured.position+=axis*dt*1.6f;
    if(s.transformMode==1)scene.featured.rotation+=axis*dt*.9f;
    if(s.transformMode==2)scene.featured.size+=axis*dt*.6f;
    for(int i=0;i<3;++i) {
        scene.featured.size[i]=clamp(scene.featured.size[i],.25f,2.8f);
        scene.featured.position[i]=clamp(scene.featured.position[i],i==1?-.5f:-25.0f,i==1?10.0f:25.0f);
    }
    Vec3 movement=camera.forward()*(float(down(GLFW_KEY_W))-float(down(GLFW_KEY_S)))+
                  camera.right()*(float(down(GLFW_KEY_D))-float(down(GLFW_KEY_A)))+
                  Vec3(0,1,0)*(float(down(GLFW_KEY_SPACE))-float(down(GLFW_KEY_LEFT_CONTROL)));
    if(length(movement)>.01f) {s.tour=false;camera.position+=normalize(movement)*dt*(down(GLFW_KEY_LEFT_SHIFT)?8.0f:3.5f);}
    camera.position.x=clamp(camera.position.x,-30,30);camera.position.z=clamp(camera.position.z,-30,30);
    camera.position.y=clamp(camera.position.y,seabedHeight(camera.position.x,camera.position.z)+.4f,14.0f);
    if(in.captured&&!s.tour) {
        double x=0,y=0;glfwGetCursorPos(w,&x,&y);
        if(in.firstMouse){in.lastX=x;in.lastY=y;in.firstMouse=false;}
        camera.yaw+=float(x-in.lastX)*.0022f;camera.pitch-=float(y-in.lastY)*.0022f;
        camera.pitch=clamp(camera.pitch,radians(-85),radians(85));in.lastX=x;in.lastY=y;
    }
    camera.fov=clamp(camera.fov-float(in.scroll)*radians(2),radians(30),radians(85));
}
void panel(UI& ui,Scene& scene,Settings& s,Camera& camera,Input& in,int w,int h,float fps,float ms,float time,int selectedFish) {
    const Vec4 white{.89f,.97f,.93f,1},muted{.54f,.76f,.76f,1},gold{.94f,.79f,.40f,1};
    ui.rect(24,24,348,91,{.025f,.115f,.15f,.85f});ui.rect(24,24,3,91,{.47f,.87f,.75f,1});
    ui.text(42,38,"LUMINOUS REEF",2.65f,white);
    ui.text(43,67,"A HAND-BUILT UNDERWATER WORLD",1.30f,muted);
    ui.text(43,89,"C++ / OPENGL / INTERACTIVE 3D",1.15f,gold);
    float x=float(w)-280,y=24;
    ui.rect(x,y,256,592,{.019f,.088f,.115f,.94f});
    ui.text(x+18,y+17,"RENDER LAB",2,white);
    ui.text(x+18,y+43,"CHANGE ONE EFFECT AT A TIME",1.12f,muted);
    y+=66;
    auto toggle=[&](const std::string& label,bool& on) {
        if(ui.button(x+12,y,232,27,label+ (on?"   ON":"  OFF"),on))on=!on;
        y+=31;
    };
    toggle("B  BLINN-PHONG",s.lighting);toggle("C  CAUSTICS",s.caustics);toggle("T  TEXTURES",s.textures);
    toggle("N  NORMAL MAPS",s.normalMaps);toggle("F  WATER FOG",s.fog);
    bool playing=!s.paused;toggle("P  ANIMATION",playing);s.paused=!playing;
    ui.text(x+18,y+5,"PROCEDURAL SCENE",1.25f,gold);y+=25;
    if(ui.button(x+12,y,232,27,"R  NEW SEED: "+std::to_string(s.seed))){++s.seed;scene.regenerate(s);}y+=31;
    const char* levels[]={"LOW","MEDIUM","HIGH"};
    if(ui.button(x+12,y,232,27,std::string("Z  DENSITY: ")+levels[s.quality])){s.quality=(s.quality+1)%3;scene.regenerate(s);}y+=31;
    if(ui.button(x+12,y,232,27,s.tour?"G  TOUR RUNNING":"G  GUIDED CAMERA TOUR",s.tour))s.tour=!s.tour;
    y+=39;
    ui.text(x+18,y,"FEATURED CORAL / TRANSFORMS",1.12f,gold);y+=20;
    const char* modes[]={"1 MOVE","2 TURN","3 SIZE"};
    for(int i=0;i<3;++i)if(ui.button(x+12+i*79,y,74,26,modes[i],s.transformMode==i))s.transformMode=i;
    y+=36;ui.text(x+18,y,"J/L: X   K/I: Y   U/O: Z",1.12f,white);y+=20;
    Vec3 values=s.transformMode==0?scene.featured.position:(s.transformMode==1?scene.featured.rotation*(180/Pi):scene.featured.size);
    ui.text(x+18,y,vectorText(values),1.14f,muted);y+=21;
    if(ui.button(x+12,y,232,25,"BACKSPACE  RESET CORAL"))scene.featured.reset();
    y+=30;
    if(ui.button(x+12,y,232,25,"F2  FOCUS FEATURED CORAL")){focusCoral(camera,scene,s);in.firstMouse=true;}
    y+=30;
    if(ui.button(x+12,y,113,25,"M WIREFRAME",s.wireframe))s.wireframe=!s.wireframe;
    if(ui.button(x+131,y,113,25,"V VSYNC",s.vsync)){s.vsync=!s.vsync;}
    ui.rect(24,float(h)-88,344,33,{.019f,.088f,.115f,.86f});
    ui.text(35,float(h)-77,"FPS "+number(fps,0)+" / "+number(ms)+" MS / SEED "+std::to_string(s.seed),1.2f,white);
    ui.rect(24,float(h)-49,float(w)-48,27,{.019f,.088f,.115f,.9f});
    ui.text(35,float(h)-40,"WASD SWIM   SPACE/CTRL UP/DOWN   TAB CURSOR   CLICK FISH TO SCAN   HOME RESET   F2 FOCUS   H UI   F12 PHOTO   ESC EXIT",1.15f,white);
    ui.text(26,132,in.captured?"DIVE MODE / TAB TO INSPECT FISH":"CLICK FISH TO SCAN / OPEN WATER TO DIVE",1.17f,gold);
    ui.text(26,154,std::to_string(scene.corals())+" CORALS / "+std::to_string(scene.fishes())+" FISH / "+std::to_string(scene.drawCalls)+" SCENE DRAWS",1.13f,muted);
    ui.text(26,174,std::to_string(scene.triangleCount/1000)+"K MESH TRIANGLES / TIME "+number(time)+" S",1.10f,muted);
    ui.rect(24,208,348,142,{.019f,.088f,.115f,.92f});ui.rect(24,208,3,142,{.94f,.79f,.40f,1});
    ui.text(42,218,"FISH FIELD GUIDE",1.45f,gold);
    if(selectedFish>=0) {
        FishInfo info=scene.fishInfo(selectedFish,time);
        ui.text(42,242,"SPECIMEN "+std::to_string(info.index+1)+" / "+info.name,1.15f,white);
        ui.text(42,260,"LENGTH "+std::to_string(info.lengthCm)+" CM / WEIGHT "+number(info.weightKg)+" KG",1.15f,white);
        ui.text(42,278,std::string("TRAIT ")+info.trait,1.15f,muted);
    } else {
        ui.text(42,244,"CLICK A FISH TO SCAN IT",1.15f,white);
        ui.text(42,264,"REVEAL ALL 4 SPECIES",1.15f,muted);
    }
    ui.text(42,316,"FIELD LOG "+std::to_string(scene.discoveredSpeciesCount())+"/4 SPECIES FOUND",1.15f,gold);
    // Crosshair only when steering. The right-hand menu remains visible for hotkey reference.
    if(in.captured) {ui.rect(float(w)/2-5,float(h)/2,11,1,muted);ui.rect(float(w)/2,float(h)/2-5,1,11,muted);}
}
int run(GLFWwindow* window,const Options& options,const char* executable) {
    Settings settings=options.settings;Camera camera;camera.reset();Input input;int selectedFish=-1;
    glfwSetWindowUserPointer(window,&input);
    glfwSetKeyCallback(window,[](GLFWwindow* w,int key,int,int action,int){
        auto* i=static_cast<Input*>(glfwGetWindowUserPointer(w));
        if(key>=0&&key<=GLFW_KEY_LAST&&action==GLFW_PRESS)i->pressed[std::size_t(key)]=true;
    });
    glfwSetMouseButtonCallback(window,[](GLFWwindow* w,int button,int action,int){
        if(button==GLFW_MOUSE_BUTTON_LEFT&&action==GLFW_PRESS)static_cast<Input*>(glfwGetWindowUserPointer(w))->click=true;
    });
    glfwSetScrollCallback(window,[](GLFWwindow* w,double,double y){static_cast<Input*>(glfwGetWindowUserPointer(w))->scroll+=y;});
    auto shaders=locateShaders(executable);Scene scene(shaders);UI ui(shaders);scene.regenerate(settings);
    glfwSwapInterval(settings.vsync?1:0);glEnable(GL_DEPTH_TEST);glEnable(GL_MULTISAMPLE);
    double previous=glfwGetTime(),statisticsStart=previous;float simulationTime=0,tourTime=0,fps=0,ms=0;
    int frame=0,statisticsFrames=0,screenshotIndex=0;
    const double runStart=previous;
    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();double now=glfwGetTime();
        float realDt=float(now-previous);previous=now;
        float dt=options.fixedDt>0?options.fixedDt:clamp(realDt,0,.05f);
        keyboard(window,input,camera,scene,settings,dt);
        if(!settings.paused)simulationTime+=dt;
        if(settings.tour) {
            if(!settings.paused)tourTime+=dt;
            float a=.55f+tourTime*.08f;
            camera.position={std::sin(a)*14.5f,4.8f+.5f*std::sin(tourTime*.12f),-6+std::cos(a)*18.0f};
            camera.lookToward({-1.0f,1.45f,-6.5f});
        }
        int width=0,height=0,windowW=0,windowH=0;
        glfwGetFramebufferSize(window,&width,&height);glfwGetWindowSize(window,&windowW,&windowH);
        if(width==0||height==0){input.clear();glfwWaitEventsTimeout(.1);continue;}
        glViewport(0,0,width,height);glClearColor(.025f,.23f,.28f,1);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        scene.render(camera,settings,simulationTime,width,height,selectedFish);
        double mx=0,my=0;glfwGetCursorPos(window,&mx,&my);
        ui.begin(windowW,windowH,input.captured?-100:float(mx),input.captured?-100:float(my),input.click&&!input.captured);
        bool previousVsync=settings.vsync;
        if(settings.ui)panel(ui,scene,settings,camera,input,windowW,windowH,fps,ms,simulationTime,selectedFish);
        if(previousVsync!=settings.vsync)glfwSwapInterval(settings.vsync?1:0);
        bool overPanel=settings.ui&&mx>=windowW-280&&my>=24&&my<=616;
        bool overFieldGuide=settings.ui&&mx>=24&&mx<=372&&my>=208&&my<=350;
        if(input.click&&!input.captured&&!overPanel&&!overFieldGuide&&!ui.handled()) {
            int fish=scene.pickFish(camera,float(mx),float(my),windowW,windowH,simulationTime);
            if(fish>=0) {selectedFish=fish;scene.discoverFish(fish);settings.tour=false;}
            else {selectedFish=-1;setCapture(window,input,true);settings.tour=false;}
        }
        if(settings.ui)ui.finish();
        if(input.pressed[GLFW_KEY_F12]) {
            auto name=std::filesystem::path("screenshots")/("reef_"+std::to_string(std::chrono::system_clock::now().time_since_epoch().count())+"_"+std::to_string(screenshotIndex++)+".bmp");
            saveScreenshot(name,width,height);std::cout<<"Saved "<<std::filesystem::absolute(name)<<'\n';
        }
        if(!options.captureDir.empty()&&frame%options.captureEvery==0) {
            std::ostringstream name;name<<"frame_"<<std::setw(5)<<std::setfill('0')<<frame<<".bmp";
            saveScreenshot(std::filesystem::path(options.captureDir)/name.str(),width,height);
        }
        if(options.frames>0&&frame==options.frames-1&&!options.capture.empty())saveScreenshot(options.capture,width,height);
        if(options.checkGL) {
            GLenum error=glGetError();if(error!=GL_NO_ERROR)throw std::runtime_error("OpenGL error "+std::to_string(error)+" at frame "+std::to_string(frame));
        }
        glfwSwapBuffers(window);input.clear();++frame;++statisticsFrames;
        double statisticsNow=glfwGetTime();
        double elapsed=statisticsNow-statisticsStart;
        if(elapsed>=.5) {
            fps=float(statisticsFrames/elapsed);ms=1000/fps;statisticsFrames=0;statisticsStart=statisticsNow;
            std::string title="Luminous Reef | "+number(fps,0)+" FPS | "+std::to_string(scene.corals())+" corals | TAB: mouse";
            glfwSetWindowTitle(window,title.c_str());
        }
        if(options.frames>0&&frame>=options.frames)break;
    }
    double duration=glfwGetTime()-runStart;
    std::cout<<"Rendered "<<frame<<" frames in "<<duration<<" s; average "<<(duration>0?frame/duration:0)
             <<" FPS. Seed "<<settings.seed<<", "<<scene.corals()<<" corals, "<<scene.drawCalls<<" scene draws, "<<scene.triangleCount<<" mesh triangles.\n";
    return 0;
}
} // namespace
int main(int argc,char** argv) {
    try {
        for(int i=1;i<argc;++i)if(std::string(argv[i])=="--help"){help();return 0;}
        Options options=parse(argc,argv);Window window(options.width,options.height);help();
        return run(window.handle,options,argv[0]); // Scene/GL resources die while the context still exists.
    } catch(const std::exception& e) {std::cerr<<"\nLUMINOUS REEF ERROR: "<<e.what()<<'\n';return 1;}
}

#include "Renderer.hpp"
#include "Update.hpp"
#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif
#include <GLFW/glfw3.h>
#include <array>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>


namespace {
struct Options {
    Settings settings;int width=1280,height=800,frames=0,captureEvery=1,inspect=-1;
    float fixedDt=0;bool checkGL=false,hasPosition=false;
    double x=0,z=0;std::string capture,captureDir;
};
void help() {
    std::cout<<"Luminous Reef - C++17 / GLFW / GLAD / Dear ImGui / modern OpenGL\n"
        "WASD move, Space/Ctrl height, Shift fast, Tab cursor, click fish to inspect.\n"
        "B lighting, C caustics, T textures, N normals, F fog, P pause, M wireframe.\n"
        "R seed, Z density, G tour, Home origin, F2 coral, H UI, V vsync, F12 screenshot.\n"
        "1 move / 2 rotate / 3 scale coral; J/L X, K/I Y, U/O Z; Backspace reset.\n"
        "Options: --width N --height N --seed N --quality low|medium|high\n"
        " --hour H --no-cycle --visibility N --position X Z --inspect SPECIES_INDEX\n"
        " --tour --no-ui --no-vsync --no-fog --no-caustics --unlit\n"
        " --no-textures --no-normals --no-bubbles --wireframe\n"
        " --frames N --fixed-dt SEC --capture FILE.bmp --capture-dir DIR\n"
        " --capture-every N --check-gl\n";
}
Options parse(int argc,char** argv) {
    Options o;
    auto value=[&](int& i){if(++i>=argc)throw std::runtime_error("Missing option value");return std::string(argv[i]);};
    auto number=[&](int& i){auto s=value(i);std::size_t n=0;double x=std::stod(s,&n);if(n!=s.size()||!std::isfinite(x))throw std::runtime_error("Invalid numeric option: "+s);return x;};
    auto integer=[&](int& i){double x=number(i);if(x<0||x>1000000000||std::floor(x)!=x)throw std::runtime_error("Expected a nonnegative integer");return int(x);};
    for(int i=1;i<argc;++i) {
        std::string a=argv[i];
        if(a=="--width")o.width=integer(i);else if(a=="--height")o.height=integer(i);
        else if(a=="--seed"){auto s=value(i);std::size_t end=0;auto seed=std::stoull(s,&end);if(end!=s.size()||seed>0xffffffffULL||s.front()=='-')throw std::runtime_error("Seed must be uint32");o.settings.seed=std::uint32_t(seed);}
        else if(a=="--frames")o.frames=integer(i);else if(a=="--capture-every")o.captureEvery=integer(i);
        else if(a=="--fixed-dt")o.fixedDt=float(number(i));else if(a=="--hour")o.settings.hour=float(number(i));
        else if(a=="--visibility")o.settings.visibility=float(number(i));else if(a=="--inspect")o.inspect=integer(i);
        else if(a=="--position"){o.x=number(i);o.z=number(i);o.hasPosition=true;}
        else if(a=="--capture")o.capture=value(i);else if(a=="--capture-dir")o.captureDir=value(i);
        else if(a=="--quality"){auto q=value(i);if(q=="low")o.settings.quality=0;else if(q=="medium")o.settings.quality=1;else if(q=="high")o.settings.quality=2;else throw std::runtime_error("Quality must be low, medium or high");}
        else if(a=="--tour")o.settings.tour=true;else if(a=="--no-ui")o.settings.ui=false;else if(a=="--no-vsync")o.settings.vsync=false;
        else if(a=="--no-fog")o.settings.fog=false;else if(a=="--no-caustics")o.settings.caustics=false;else if(a=="--unlit")o.settings.lighting=false;
        else if(a=="--no-textures")o.settings.textures=false;else if(a=="--no-normals")o.settings.normalMaps=false;
        else if(a=="--no-bubbles")o.settings.bubbles=false;else if(a=="--wireframe")o.settings.wireframe=true;
        else if(a=="--no-cycle")o.settings.dayCycle=false;else if(a=="--check-gl")o.checkGL=true;
        else throw std::runtime_error("Unknown option: "+a+" (use --help)");
    }
    if(o.width<320||o.height<240||o.width>7680||o.height>4320||o.fixedDt<0||o.fixedDt>.25f||o.captureEvery<1||
       o.settings.hour<0||o.settings.hour>=24||o.settings.visibility<24||o.settings.visibility>96||o.inspect>=SpeciesCount||
       std::abs(o.x)>1e12||std::abs(o.z)>1e12)throw std::runtime_error("Option is outside its supported range");
    if(!o.capture.empty()&&o.frames==0)throw std::runtime_error("--capture requires --frames N; use F12 interactively");
    return o;
}
struct Window {
    GLFWwindow* handle=nullptr;
    Window(int width,int height) {
        glfwSetErrorCallback([](int code,const char* text){std::cerr<<"GLFW "<<code<<": "<<text<<'\n';});
        if(!glfwInit())throw std::runtime_error("GLFW initialization failed. Check the GPU driver.");
        const int versions[][3]={{4,1,4},{3,3,4},{3,3,0}};
        for(const auto& v:versions) {
            glfwDefaultWindowHints();glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,v[0]);glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,v[1]);
            glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);glfwWindowHint(GLFW_SAMPLES,v[2]);
#ifdef __APPLE__
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT,GLFW_TRUE);
#endif
            handle=glfwCreateWindow(width,height,"Luminous Reef - complete project",nullptr,nullptr);if(handle)break;
        }
        if(!handle){glfwTerminate();throw std::runtime_error("An OpenGL 3.3+ Core graphics driver is required");}
        glfwMakeContextCurrent(handle);
        if(!gladLoadGL(reinterpret_cast<GLADloadfunc>(glfwGetProcAddress))){glfwDestroyWindow(handle);handle=nullptr;glfwTerminate();throw std::runtime_error("GLAD loading failed");}
        std::cout<<"OpenGL: "<<glGetString(GL_VERSION)<<"\nGPU: "<<glGetString(GL_RENDERER)<<'\n';
    }
    ~Window(){if(handle)glfwDestroyWindow(handle);glfwTerminate();}
};
struct Input {
    std::array<bool,GLFW_KEY_LAST+1> pressed{};bool click=false,captured=false,firstMouse=true;
    double lastX=0,lastY=0,scroll=0;
    void clear(){pressed.fill(false);click=false;scroll=0;}
};
void capture(GLFWwindow* w,Input& in,bool on) {
    in.captured=on;in.firstMouse=true;glfwSetInputMode(w,GLFW_CURSOR,on?GLFW_CURSOR_DISABLED:GLFW_CURSOR_NORMAL);
    if(glfwRawMouseMotionSupported())glfwSetInputMode(w,GLFW_RAW_MOUSE_MOTION,on?GLFW_TRUE:GLFW_FALSE);
}
void gather(GLFWwindow* w,Input& input,InputFrame& frame,bool keyboardBlocked,bool mouseBlocked) {
    auto down=[&](int key){return glfwGetKey(w,key)==GLFW_PRESS;};
    if(!keyboardBlocked) {
        const std::pair<int,Action> mapping[]={{GLFW_KEY_B,Action::Lighting},{GLFW_KEY_C,Action::Caustics},{GLFW_KEY_T,Action::Textures},
            {GLFW_KEY_N,Action::Normals},{GLFW_KEY_F,Action::Fog},{GLFW_KEY_P,Action::Pause},{GLFW_KEY_M,Action::Wireframe},
            {GLFW_KEY_H,Action::UI},{GLFW_KEY_G,Action::Tour},{GLFW_KEY_V,Action::Vsync},{GLFW_KEY_R,Action::NewSeed},
            {GLFW_KEY_Z,Action::Quality},{GLFW_KEY_HOME,Action::Home},{GLFW_KEY_F2,Action::Focus},
            {GLFW_KEY_BACKSPACE,Action::ResetCoral},{GLFW_KEY_1,Action::MoveMode},{GLFW_KEY_2,Action::RotateMode},{GLFW_KEY_3,Action::ScaleMode}};
        for(const auto& p:mapping)if(input.pressed[std::size_t(p.first)])frame.press(p.second);
        frame.move={float(down(GLFW_KEY_D))-float(down(GLFW_KEY_A)),float(down(GLFW_KEY_SPACE))-float(down(GLFW_KEY_LEFT_CONTROL)),float(down(GLFW_KEY_W))-float(down(GLFW_KEY_S))};
        frame.transformAxis={float(down(GLFW_KEY_L))-float(down(GLFW_KEY_J)),float(down(GLFW_KEY_I))-float(down(GLFW_KEY_K)),float(down(GLFW_KEY_O))-float(down(GLFW_KEY_U))};
        frame.fast=down(GLFW_KEY_LEFT_SHIFT);
    }
    if(input.captured) {
        double x=0,y=0;glfwGetCursorPos(w,&x,&y);
        if(input.firstMouse){input.lastX=x;input.lastY=y;input.firstMouse=false;}
        frame.looking=true;frame.mouseDelta={float(x-input.lastX),float(y-input.lastY)};input.lastX=x;input.lastY=y;
    }
    if(!mouseBlocked||input.captured)frame.scroll=float(input.scroll);
}
int run(GLFWwindow* window,const Options& o,const char* executable) {
    Scene scene;scene.settings=o.settings;Update update;update.initialize(scene);
    if(o.hasPosition) {
        scene.origin={static_cast<std::int64_t>(std::floor((o.x+12)/ChunkSize)),static_cast<std::int64_t>(std::floor((o.z+12)/ChunkSize))};
        scene.camera.position={float(o.x-double(scene.origin.first)*ChunkSize),5,float(o.z-double(scene.origin.second)*ChunkSize)};
        scene.camera.position.y=scene.floorAt(scene.camera.position)+5;update.regenerate(scene);
    }
    if(o.inspect>=0){scene.guideSpecies=o.inspect;scene.showFishInfo=true;}
    Input input;glfwSetWindowUserPointer(window,&input);
    glfwSetKeyCallback(window,[](GLFWwindow* w,int k,int,int action,int){if(k>=0&&k<=GLFW_KEY_LAST&&action==GLFW_PRESS)static_cast<Input*>(glfwGetWindowUserPointer(w))->pressed[std::size_t(k)]=true;});
    glfwSetMouseButtonCallback(window,[](GLFWwindow* w,int b,int action,int){if(b==GLFW_MOUSE_BUTTON_LEFT&&action==GLFW_PRESS)static_cast<Input*>(glfwGetWindowUserPointer(w))->click=true;});
    glfwSetScrollCallback(window,[](GLFWwindow* w,double,double y){static_cast<Input*>(glfwGetWindowUserPointer(w))->scroll+=y;});
    glfwSetWindowFocusCallback(window,[](GLFWwindow* w,int focused){if(!focused){auto& in=*static_cast<Input*>(glfwGetWindowUserPointer(w));capture(w,in,false);in.clear();}});
    auto shaders=locateShaders(executable);Renderer renderer(shaders,shaders.parent_path()/"assets");Ui ui;
    float sx=1,sy=1;glfwGetWindowContentScale(window,&sx,&sy);renderer.initializeUI(window,std::min(sx,sy));
    bool lastVsync=scene.settings.vsync;glfwSwapInterval(lastVsync?1:0);
    double previous=glfwGetTime(),runStart=previous,statsStart=previous;int frame=0,statsFrames=0;float fps=0,ms=0;
    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();double now=glfwGetTime();float dt=o.fixedDt>0?o.fixedDt:clamp(float(now-previous),0,.05f);previous=now;
        int width=0,height=0,windowW=0,windowH=0;glfwGetFramebufferSize(window,&width,&height);glfwGetWindowSize(window,&windowW,&windowH);
        if(width<=0||height<=0||windowW<=0||windowH<=0){input.clear();glfwWaitEventsTimeout(.1);continue;}
        if(input.pressed[GLFW_KEY_ESCAPE])break;
        if(input.pressed[GLFW_KEY_TAB])capture(window,input,!input.captured);
        renderer.beginUI();InputFrame commands=ui.build(scene,renderer.stats(),renderer.photos(),input.captured,fps,ms);
        gather(window,input,commands,renderer.wantsKeyboard(),renderer.wantsMouse());
        if(commands.pressed(Action::Home)||commands.pressed(Action::Focus)||commands.pressed(Action::Tour)){capture(window,input,false);commands.looking=false;}
        update.step(scene,commands,dt);
        if(lastVsync!=scene.settings.vsync){lastVsync=scene.settings.vsync;glfwSwapInterval(lastVsync?1:0);}
        renderer.render(scene,width,height);
        if(input.click&&!input.captured&&!renderer.wantsMouse()) {
            double mx=0,my=0;glfwGetCursorPos(window,&mx,&my);
            int x=int(std::floor(mx*width/windowW)),y=height-1-int(std::floor(my*height/windowH));
            int picked=renderer.pick(scene,x,y);
            if(picked>=0)update.selectFish(scene,picked);else {update.selectFish(scene,-1);capture(window,input,true);scene.settings.tour=false;}
        }
        renderer.endUI();
        if(input.pressed[GLFW_KEY_F12]) {
            auto stamp=std::chrono::system_clock::now().time_since_epoch().count();
            auto path=std::filesystem::path("screenshots")/("reef_"+std::to_string(stamp)+".bmp");renderer.screenshot(path);std::cout<<"Saved "<<path<<'\n';
        }
        if(!o.captureDir.empty()&&frame%o.captureEvery==0){std::ostringstream file;file<<"frame_"<<std::setw(5)<<std::setfill('0')<<frame<<".bmp";renderer.screenshot(std::filesystem::path(o.captureDir)/file.str());}
        if(o.frames>0&&frame==o.frames-1&&!o.capture.empty())renderer.screenshot(o.capture);
        if(o.checkGL)Renderer::checkErrors();
        glfwSwapBuffers(window);input.clear();++frame;++statsFrames;
        double elapsed=glfwGetTime()-statsStart;
        if(elapsed>=.5){fps=float(statsFrames/elapsed);ms=1000/fps;statsFrames=0;statsStart=glfwGetTime();}
        if(o.frames>0&&frame>=o.frames)break;
    }
    auto stats=renderer.stats();
    std::cout<<"Frames: "<<frame<<" | elapsed wall seconds: "<<glfwGetTime()-runStart
        <<" | last frame draws: "<<stats.draws<<" | triangles: "<<stats.triangles<<" | loaded chunks: "<<scene.chunks.size()<<'\n';
    return 0;
}
}
int main(int argc,char** argv) {
    try {
        for(int i=1;i<argc;++i)if(std::string(argv[i])=="--help"){help();return 0;}
        auto options=parse(argc,argv);Window window(options.width,options.height);help();return run(window.handle,options,argv[0]);
    } catch(const std::exception& e){std::cerr<<"\nLUMINOUS REEF ERROR: "<<e.what()<<'\n';return 1;}
}

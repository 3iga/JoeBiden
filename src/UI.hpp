#pragma once
#include "Graphics.hpp"
namespace reef {
class UI {
    struct UIVertex { Vec2 position;Vec4 color; };
    Shader shader;
    GLuint vao=0,vbo=0;
    std::vector<UIVertex> vertices;
    float mouseX=0,mouseY=0;
    bool clicked=false,consumed=false;
    int width=1280,height=720;
public:
    explicit UI(const std::filesystem::path& path);
    ~UI();
    void begin(int w,int h,float mx,float my,bool mouseClick);
    void rect(float x,float y,float w,float h,Vec4 color);
    void text(float x,float y,const std::string& text,float size,Vec4 color);
    bool button(float x,float y,float w,float h,const std::string& label,bool active=false);
    bool hit(float x,float y,float w,float h) const;
    bool handled() const { return consumed; }
    void finish();
};
} // namespace reef

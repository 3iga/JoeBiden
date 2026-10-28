#include "UI.hpp"
#include <array>
#include <cctype>
namespace reef {
// A tiny hand-defined 5x7 bitmap alphabet drawn as colored rectangles, not a font file.
// Text and buttons are ordinary triangles, so no GUI or text library is required.
static std::array<unsigned char,7> glyph(char c) {
    switch(std::toupper(static_cast<unsigned char>(c))) {
    case 'A':return {14,17,17,31,17,17,17};case 'B':return {30,17,17,30,17,17,30};
    case 'C':return {14,17,16,16,16,17,14};case 'D':return {30,17,17,17,17,17,30};
    case 'E':return {31,16,16,30,16,16,31};case 'F':return {31,16,16,30,16,16,16};
    case 'G':return {14,17,16,23,17,17,15};case 'H':return {17,17,17,31,17,17,17};
    case 'I':return {31,4,4,4,4,4,31};case 'J':return {7,2,2,2,2,18,12};
    case 'K':return {17,18,20,24,20,18,17};case 'L':return {16,16,16,16,16,16,31};
    case 'M':return {17,27,21,21,17,17,17};case 'N':return {17,25,21,19,17,17,17};
    case 'O':return {14,17,17,17,17,17,14};case 'P':return {30,17,17,30,16,16,16};
    case 'Q':return {14,17,17,17,21,18,13};case 'R':return {30,17,17,30,20,18,17};
    case 'S':return {15,16,16,14,1,1,30};case 'T':return {31,4,4,4,4,4,4};
    case 'U':return {17,17,17,17,17,17,14};case 'V':return {17,17,17,17,17,10,4};
    case 'W':return {17,17,17,21,21,21,10};case 'X':return {17,17,10,4,10,17,17};
    case 'Y':return {17,17,10,4,4,4,4};case 'Z':return {31,1,2,4,8,16,31};
    case '0':return {14,17,19,21,25,17,14};case '1':return {4,12,4,4,4,4,14};
    case '2':return {14,17,1,2,4,8,31};case '3':return {30,1,1,14,1,1,30};
    case '4':return {2,6,10,18,31,2,2};case '5':return {31,16,16,30,1,1,30};
    case '6':return {14,16,16,30,17,17,14};case '7':return {31,1,2,4,8,8,8};
    case '8':return {14,17,17,14,17,17,14};case '9':return {14,17,17,15,1,1,14};
    case ':':return {0,4,4,0,4,4,0};case '.':return {0,0,0,0,0,6,6};
    case ',':return {0,0,0,0,6,6,4};case '-':return {0,0,0,31,0,0,0};
    case '+':return {0,4,4,31,4,4,0};case '/':return {1,2,2,4,8,8,16};
    case '[':return {14,8,8,8,8,8,14};case ']':return {14,2,2,2,2,2,14};
    case '(':return {2,4,8,8,8,4,2};case ')':return {8,4,2,2,2,4,8};
    case '=':return {0,0,31,0,31,0,0};case '>':return {16,8,4,2,4,8,16};
    case '<':return {1,2,4,8,4,2,1};case '%':return {17,2,4,8,16,17,0};
    case '|':return {4,4,4,4,4,4,4};case '_':return {0,0,0,0,0,0,31};
    default:return {0,0,0,0,0,0,0};
    }
}
UI::UI(const std::filesystem::path& p):shader(p/"ui.vert",p/"ui.frag") {
    glGenVertexArrays(1,&vao);glGenBuffers(1,&vbo);glBindVertexArray(vao);glBindBuffer(GL_ARRAY_BUFFER,vbo);
    glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,sizeof(UIVertex),reinterpret_cast<void*>(offsetof(UIVertex,position)));
    glVertexAttribPointer(1,4,GL_FLOAT,GL_FALSE,sizeof(UIVertex),reinterpret_cast<void*>(offsetof(UIVertex,color)));
    glEnableVertexAttribArray(0);glEnableVertexAttribArray(1);glBindVertexArray(0);
}
UI::~UI() { glDeleteBuffers(1,&vbo);glDeleteVertexArrays(1,&vao); }
void UI::begin(int w,int h,float mx,float my,bool mouseClick) {
    width=w;height=h;mouseX=mx;mouseY=my;clicked=mouseClick;consumed=false;vertices.clear();
}
void UI::rect(float x,float y,float w,float h,Vec4 c) {
    vertices.insert(vertices.end(),{{{x,y},c},{{x+w,y},c},{{x+w,y+h},c},{{x,y},c},{{x+w,y+h},c},{{x,y+h},c}});
}
void UI::text(float x,float y,const std::string& s,float size,Vec4 c) {
    float origin=x;
    for(char ch:s) {
        if(ch=='\n'){x=origin;y+=size*10;continue;}
        auto rows=glyph(ch);
        for(int j=0;j<7;++j)for(int i=0;i<5;++i)if(rows[j]&(1<<(4-i)))rect(x+i*size,y+j*size,size,size,c);
        x+=size*6;
    }
}
bool UI::hit(float x,float y,float w,float h) const { return mouseX>=x&&mouseX<=x+w&&mouseY>=y&&mouseY<=y+h; }
bool UI::button(float x,float y,float w,float h,const std::string& label,bool active) {
    bool hover=hit(x,y,w,h);
    Vec4 color=active?Vec4{.12f,.39f,.38f,.96f}:Vec4{.045f,.16f,.20f,.95f};
    if(hover){color.x+=.045f;color.y+=.065f;color.z+=.065f;}
    rect(x,y,w,h,color);
    if(active)rect(x,y,3,h,{.50f,.94f,.77f,1});
    text(x+10,y+(h-9.1f)*.5f,label,1.3f,{.88f,.96f,.93f,1});
    if(clicked&&hover){consumed=true;return true;}return false;
}
void UI::finish() {
    if(vertices.empty())return;
    glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);glDisable(GL_DEPTH_TEST);glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    shader.use();shader.set("uWindow",Vec2{float(width),float(height)});
    glBindVertexArray(vao);glBindBuffer(GL_ARRAY_BUFFER,vbo);
    glBufferData(GL_ARRAY_BUFFER,static_cast<GLsizeiptr>(vertices.size()*sizeof(UIVertex)),vertices.data(),GL_STREAM_DRAW);
    glDrawArrays(GL_TRIANGLES,0,static_cast<GLsizei>(vertices.size()));glBindVertexArray(0);
    glDisable(GL_BLEND);glEnable(GL_DEPTH_TEST);
}
} // namespace reef

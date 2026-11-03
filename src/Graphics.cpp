#include "Graphics.hpp"
#include "Procedural.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <array>

namespace reef {
static std::string readText(const std::filesystem::path& path) {
    std::ifstream f(path);
    if(!f) throw std::runtime_error("Cannot read shader: "+path.string());
    std::ostringstream s;s<<f.rdbuf();return s.str();
}
static GLuint compile(GLenum kind,const std::filesystem::path& path) {
    std::string text=readText(path);const char* c=text.c_str();
    GLuint shader=glCreateShader(kind);glShaderSource(shader,1,&c,nullptr);glCompileShader(shader);
    GLint ok=0;glGetShaderiv(shader,GL_COMPILE_STATUS,&ok);
    if(!ok) {
        GLint n=0;glGetShaderiv(shader,GL_INFO_LOG_LENGTH,&n);
        std::string log(std::max(n,1), '\0');glGetShaderInfoLog(shader,n,nullptr,log.data());
        glDeleteShader(shader);throw std::runtime_error(path.string()+"\n"+log);
    }
    return shader;
}
Shader::Shader(const std::filesystem::path& v,const std::filesystem::path& f) {
    GLuint vs=compile(GL_VERTEX_SHADER,v),fs=0;
    try { fs=compile(GL_FRAGMENT_SHADER,f); }
    catch(...) { glDeleteShader(vs);throw; }
    id_=glCreateProgram();glAttachShader(id_,vs);glAttachShader(id_,fs);glLinkProgram(id_);
    glDeleteShader(vs);glDeleteShader(fs);
    GLint ok=0;glGetProgramiv(id_,GL_LINK_STATUS,&ok);
    if(!ok) {
        GLint n=0;glGetProgramiv(id_,GL_INFO_LOG_LENGTH,&n);
        std::string log(std::max(n,1),'\0');glGetProgramInfoLog(id_,n,nullptr,log.data());
        glDeleteProgram(id_);id_=0;throw std::runtime_error("Shader link failed: "+log);
    }
}
Shader::~Shader() { if(id_) glDeleteProgram(id_); }
GLint Shader::location(const std::string& name) const {
    auto it=locations.find(name);if(it!=locations.end()) return it->second;
    GLint p=glGetUniformLocation(id_,name.c_str());locations[name]=p;return p;
}
void Shader::set(const std::string& n,int x) const { glUniform1i(location(n),x); }
void Shader::set(const std::string& n,float x) const { glUniform1f(location(n),x); }
void Shader::set(const std::string& n,Vec2 x) const { glUniform2f(location(n),x.x,x.y); }
void Shader::set(const std::string& n,Vec3 x) const { glUniform3f(location(n),x.x,x.y,x.z); }
void Shader::set(const std::string& n,Vec4 x) const { glUniform4f(location(n),x.x,x.y,x.z,x.w); }
void Shader::set(const std::string& n,const Mat4& x) const { glUniformMatrix4fv(location(n),1,GL_FALSE,x.data()); }
Mesh::Mesh(const MeshData& data) {
    indexCount=static_cast<GLsizei>(data.indices.size());
    glGenVertexArrays(1,&vao);glGenBuffers(1,&vbo);glGenBuffers(1,&ebo);glGenBuffers(1,&instanceBuffer);
    glBindVertexArray(vao);glBindBuffer(GL_ARRAY_BUFFER,vbo);
    glBufferData(GL_ARRAY_BUFFER,static_cast<GLsizeiptr>(data.vertices.size()*sizeof(Vertex)),data.vertices.data(),GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,static_cast<GLsizeiptr>(data.indices.size()*sizeof(unsigned)),data.indices.data(),GL_STATIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),reinterpret_cast<void*>(offsetof(Vertex,position)));
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),reinterpret_cast<void*>(offsetof(Vertex,normal)));
    glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,sizeof(Vertex),reinterpret_cast<void*>(offsetof(Vertex,uv)));
    glEnableVertexAttribArray(0);glEnableVertexAttribArray(1);glEnableVertexAttribArray(2);
    // A matrix occupies four consecutive vertex attributes. Divisor 1 = per instance.
    glBindBuffer(GL_ARRAY_BUFFER,instanceBuffer);
    for(unsigned i=0;i<4;++i) {
        glEnableVertexAttribArray(3+i);
        glVertexAttribPointer(3+i,4,GL_FLOAT,GL_FALSE,sizeof(Instance),reinterpret_cast<void*>(i*4*sizeof(float)));
        glVertexAttribDivisor(3+i,1);
    }
    glEnableVertexAttribArray(7);
    glVertexAttribPointer(7,4,GL_FLOAT,GL_FALSE,sizeof(Instance),reinterpret_cast<void*>(offsetof(Instance,tint)));
    glVertexAttribDivisor(7,1);
    glBindVertexArray(0);
    // Also provide valid attribute data for non-instanced draws.
    setInstances({Instance{Mat4::identity(),{1,1,1,1}}});
}
Mesh::~Mesh() {
    glDeleteBuffers(1,&instanceBuffer);glDeleteBuffers(1,&ebo);glDeleteBuffers(1,&vbo);glDeleteVertexArrays(1,&vao);
}
void Mesh::setInstances(const std::vector<Instance>& data) {
    instanceCount=static_cast<GLsizei>(data.size());
    glBindBuffer(GL_ARRAY_BUFFER,instanceBuffer);
    // Do not leave an empty enabled instance buffer; still issue zero draws for zero instances.
    Instance dummy{Mat4::identity(),{1,1,1,1}};
    glBufferData(GL_ARRAY_BUFFER,static_cast<GLsizeiptr>(std::max(std::size_t(1),data.size())*sizeof(Instance)),data.empty()?&dummy:data.data(),GL_STATIC_DRAW);
}
void Mesh::draw() const { glBindVertexArray(vao);glDrawElements(GL_TRIANGLES,indexCount,GL_UNSIGNED_INT,nullptr); }
void Mesh::drawInstances() const {
    if(instanceCount==0) return;
    glBindVertexArray(vao);glDrawElementsInstanced(GL_TRIANGLES,indexCount,GL_UNSIGNED_INT,nullptr,instanceCount);
}
void Texture::uploadRGB(int size,const std::vector<unsigned char>& pixels) {
    if(!id) glGenTextures(1,&id);
    glBindTexture(GL_TEXTURE_2D,id);glPixelStorei(GL_UNPACK_ALIGNMENT,1);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGB8,size,size,0,GL_RGB,GL_UNSIGNED_BYTE,pixels.data());
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
}
void Texture::bind(int unit) const { glActiveTexture(GL_TEXTURE0+unit);glBindTexture(GL_TEXTURE_2D,id); }
static unsigned char byte(float v) { return static_cast<unsigned char>(clamp(v,0,1)*255+.5f); }
void makeMaterial(Material& m,int kind) {
    // kind: 0 sand, 1 coral, 2 rock, 3 fish, 4 leaf, 5 plain.
    constexpr int N=256;
    std::vector<float> heights(N*N);
    std::vector<unsigned char> color(N*N*3),normal(N*N*3);
    for(int y=0;y<N;++y) for(int x=0;x<N;++x) {
        float u=float(x)/N,v=float(y)/N,a=2*Pi*u,b=2*Pi*v;
        float fine=.5f+.5f*std::sin(43*a+2*std::sin(3*b))*std::sin(37*b+std::sin(2*a));
        float h=.5f;Vec3 c(1);
        if(kind==0) { h=.5f+.33f*std::sin(8*b+1.2f*std::sin(2*a))+.07f*fine;c=Vec3(.86f,.83f,.67f)*(.83f+.19f*h); }
        if(kind==1) { h=.45f+.30f*std::sin(12*a+std::sin(3*b))*std::sin(12*b)+.10f*fine;c=Vec3(.88f+.12f*h); }
        if(kind==2) { h=.5f+.18f*std::sin(3*a+std::cos(2*b))+.15f*std::cos(5*b+std::sin(4*a))+.1f*fine;c=Vec3(.47f,.54f,.48f)*(.7f+.45f*h); }
        if(kind==3) {
            float scales=.5f+.5f*std::sin(32*a+2.2f*std::sin(5*b))*std::sin(24*b+1.4f*std::sin(3*a));
            h=.5f+.035f*std::sin(18*a)*std::sin(12*b)+.025f*scales;
            c=Vec3(.82f,.94f,.91f)*(.88f+.12f*scales);
        }
        if(kind==4) { h=.5f+.2f*std::sin(6*a);c=Vec3(.48f,.82f,.45f)*(.80f+.2f*h); }
        heights[y*N+x]=h;int k=(y*N+x)*3;
        color[k]=byte(c.x);color[k+1]=byte(c.y);color[k+2]=byte(c.z);
    }
    float strength=kind==0 ? 3.5f : (kind==1 ? 1.7f : 2.5f);
    for(int y=0;y<N;++y) for(int x=0;x<N;++x) {
        float dx=heights[y*N+(x+1)%N]-heights[y*N+(x+N-1)%N];
        float dy=heights[((y+1)%N)*N+x]-heights[((y+N-1)%N)*N+x];
        Vec3 n=normalize(Vec3(-dx*strength,-dy*strength,1));int k=(y*N+x)*3;
        normal[k]=byte(n.x*.5f+.5f);normal[k+1]=byte(n.y*.5f+.5f);normal[k+2]=byte(n.z*.5f+.5f);
    }
    m.diffuse.uploadRGB(N,color);m.normal.uploadRGB(N,normal);
    m.shininess=kind==3?72.0f:24.0f;m.specular=kind==3?.65f:.22f;m.uvScale=kind==0?24.0f:1.0f;
}
void makeCaustics(Texture& texture) {
    constexpr int N=256,C=7;Random rng(2981);
    std::array<Vec2,C*C> seeds;
    for(auto& s:seeds) s={rng.range(.16f,.84f),rng.range(.16f,.84f)};
    std::vector<unsigned char> pixels(N*N*3);
    for(int y=0;y<N;++y) for(int x=0;x<N;++x) {
        float u=float(x)/N,v=float(y)/N;
        float px=C*(u+.032f*std::sin(v*2*Pi)),py=C*(v+.032f*std::sin(u*4*Pi));
        int ix=int(std::floor(px)),iy=int(std::floor(py));float d1=100,d2=100;
        for(int j=-1;j<=1;++j) for(int i=-1;i<=1;++i) {
            int gx=ix+i,gy=iy+j;
            Vec2 s=seeds[((gy%C+C)%C)*C+(gx%C+C)%C];
            float dx=gx+s.x-px,dy=gy+s.y-py,d=std::sqrt(dx*dx+dy*dy);
            if(d<d1) { d2=d1;d1=d; } else if(d<d2) d2=d;
        }
        float light=std::exp(-(d2-d1)*26.0f);unsigned char c=byte(light);
        int k=(y*N+x)*3;pixels[k]=pixels[k+1]=pixels[k+2]=c;
    }
    texture.uploadRGB(N,pixels);
}
void saveScreenshot(const std::filesystem::path& path,int width,int height) {
    // 24-bit BMP, bottom-up rows: glReadPixels already gives the needed row order.
    if(width<=0||height<=0) throw std::runtime_error("Cannot capture a zero-sized framebuffer");
    if(!path.parent_path().empty()) std::filesystem::create_directories(path.parent_path());
    std::vector<unsigned char> rgb(std::size_t(width)*height*3);
    glPixelStorei(GL_PACK_ALIGNMENT,1);glReadBuffer(GL_BACK);
    glReadPixels(0,0,width,height,GL_RGB,GL_UNSIGNED_BYTE,rgb.data());
    unsigned stride=(unsigned(width)*3+3)&~3u,size=54+stride*unsigned(height);
    std::ofstream out(path,std::ios::binary);if(!out) throw std::runtime_error("Cannot save "+path.string());
    auto u16=[&](unsigned v){out.put(char(v&255));out.put(char((v>>8)&255));};
    auto u32=[&](unsigned v){for(int i=0;i<4;++i)out.put(char((v>>(8*i))&255));};
    out.put('B');out.put('M');u32(size);u16(0);u16(0);u32(54);u32(40);u32(unsigned(width));u32(unsigned(height));
    u16(1);u16(24);u32(0);u32(stride*unsigned(height));u32(2835);u32(2835);u32(0);u32(0);
    for(int y=0;y<height;++y) {
        for(int x=0;x<width;++x) {
            std::size_t k=(std::size_t(y)*width+x)*3;
            out.put(char(rgb[k+2]));out.put(char(rgb[k+1]));out.put(char(rgb[k]));
        }
        for(unsigned p=unsigned(width)*3;p<stride;++p)out.put(0);
    }
    if(!out) throw std::runtime_error("Failed while writing "+path.string());
}
std::filesystem::path locateShaders(const char* executable) {
    namespace fs=std::filesystem;
    std::vector<fs::path> paths={fs::absolute(executable).parent_path()/"shaders",fs::current_path()/"shaders"};
#ifdef REEF_SOURCE_DIR
    paths.emplace_back(fs::path(REEF_SOURCE_DIR)/"shaders");
#endif
    for(const auto& p:paths) if(fs::exists(p/"reef.vert"))return p;
    throw std::runtime_error("Shader folder not found. Keep shaders/ next to LuminousReef.exe.");
}
} // namespace reef

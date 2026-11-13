#pragma once
#include <glad/gl.h>
#include "MeshData.hpp"
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

struct Shader {
    GLuint id_=0;
    mutable std::unordered_map<std::string,GLint> locations;
    GLint location(const std::string& name) const;
public:
    Shader(const std::filesystem::path& vertex,const std::filesystem::path& fragment);
    ~Shader();
    Shader(const Shader&)=delete;Shader& operator=(const Shader&)=delete;
    bool valid() const { return id_ != 0; }
    void use() const { glUseProgram(id_); }
    void set(const std::string& name,int x) const;
    void set(const std::string& name,float x) const;
    void set(const std::string& name,Vec2 x) const;
    void set(const std::string& name,Vec3 x) const;
    void set(const std::string& name,Vec4 x) const;
    void set(const std::string& name,const Mat4& x) const;
};
struct Mesh {
    bool twoSided=false;
    GLuint vao=0,vbo=0,ebo=0,instanceBuffer=0;
    GLsizei indexCount=0,instanceCount=0;
public:
    explicit Mesh(const MeshData& data);
    ~Mesh();
    Mesh(const Mesh&)=delete;Mesh& operator=(const Mesh&)=delete;
    void setInstances(const std::vector<Instance>& data);
    int triangles() const { return indexCount/3; }
    int instances() const { return instanceCount; }
};
struct Texture {
    GLuint id=0;
    int width=0,height=0;
    Texture()=default;
    ~Texture() { if(id) glDeleteTextures(1,&id); }
    Texture(const Texture&)=delete;Texture& operator=(const Texture&)=delete;
    void uploadRGB(int size,const std::vector<unsigned char>& pixels);
    void bind(int unit) const;
    bool loadBMP(const std::filesystem::path& path,std::string& error);
};
struct Material {
    Texture diffuse,normal;
    float shininess=32, specular=.25f,uvScale=1;
};
void makeMaterial(Material& material,int kind);
void makeCaustics(Texture& texture);
bool saveScreenshot(const std::filesystem::path& path,int width,int height);

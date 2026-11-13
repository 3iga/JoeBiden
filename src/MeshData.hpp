#pragma once
#include "MathSupport.hpp"
#include <vector>

struct Vertex { Vec3 position;
Vec3 normal;
Vec2 uv;
};
struct MeshData { bool twoSided=false;
std::vector<Vertex> vertices;
std::vector<unsigned> indices;
};
struct Instance { Mat4 model;
Vec4 tint;
};

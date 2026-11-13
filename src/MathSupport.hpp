#pragma once
// GLM is the math implementation. These aliases and composition helpers are not
// a replacement math library; they make the old Demo's coordinate conventions explicit.
#ifndef GLM_FORCE_CTOR_INIT
#define GLM_FORCE_CTOR_INIT
#endif
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>
#include <cmath>
#include <cstddef>

using Vec2 = glm::vec2;
using Vec3 = glm::vec3;
using Vec4 = glm::vec4;
using Mat4 = glm::mat4;
constexpr float Pi = 3.14159265358979323846f;
inline float radians(float degrees) {
    return glm::radians(degrees);
}
inline float clamp(float value, float low, float high) {
    return glm::clamp(value, low, high);
}
inline Vec3 safeNormalize(Vec3 v) {
    const float squared = glm::dot(v, v);
    return squared > 1e-14f ? v / std::sqrt(squared) : Vec3(0, 1, 0);
}
inline Mat4 translate(Vec3 p) {
    return glm::translate(Mat4(1.0f), p);
}
inline Mat4 scale(Vec3 s) {
    return glm::scale(Mat4(1.0f), s);
}
inline Mat4 rotateX(float a) {
    return glm::rotate(Mat4(1.0f), a, Vec3(1, 0, 0));
}
inline Mat4 rotateY(float a) {
    return glm::rotate(Mat4(1.0f), a, Vec3(0, 1, 0));
}
inline Mat4 rotateZ(float a) {
    return glm::rotate(Mat4(1.0f), a, Vec3(0, 0, 1));
}
inline Mat4 compose(Vec3 p, Vec3 a, Vec3 s) {
    return translate(p) * rotateZ(a.z) * rotateY(a.y) * rotateX(a.x) * scale(s);
}
inline Vec3 transformPoint(const Mat4& m, Vec3 p) {
    return Vec3(m * Vec4(p, 1.0f));
}
// Right-handed, OpenGL [-1,+1] depth. Do not define GLM_FORCE_DEPTH_ZERO_TO_ONE.
inline Mat4 perspective(float fov, float aspect, float nearZ, float farZ) {
    return glm::perspective(fov, aspect, nearZ, farZ);
}
inline Mat4 lookAt(Vec3 eye, Vec3 target, Vec3 up = Vec3(0, 1, 0)) {
    return glm::lookAt(eye, target, up);
}

inline Vec3 mix(Vec3 a, Vec3 b, float t) {
    return glm::mix(a, b, t);
}

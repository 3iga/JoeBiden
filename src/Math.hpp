#pragma once
// Small, explicit column-major math types: no engine or hidden transform stack.
#include <algorithm>
#include <cmath>
#include <cstddef>

namespace reef {
constexpr float Pi = 3.14159265358979323846f;
inline float radians(float degrees) { return degrees * Pi / 180.0f; }
inline float clamp(float x, float a, float b) { return std::max(a, std::min(b, x)); }
struct Vec2 { float x=0, y=0; };
struct Vec3 {
    float x=0, y=0, z=0;
    Vec3() = default;
    explicit Vec3(float v) : x(v), y(v), z(v) {}
    Vec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
    float& operator[](int i) { return i == 0 ? x : (i == 1 ? y : z); }
    float operator[](int i) const { return i == 0 ? x : (i == 1 ? y : z); }
};
struct Vec4 { float x=0, y=0, z=0, w=1; };
inline Vec3 operator+(Vec3 a, Vec3 b) { return {a.x+b.x,a.y+b.y,a.z+b.z}; }
inline Vec3 operator-(Vec3 a, Vec3 b) { return {a.x-b.x,a.y-b.y,a.z-b.z}; }
inline Vec3 operator-(Vec3 a) { return {-a.x,-a.y,-a.z}; }
inline Vec3 operator*(Vec3 a, float s) { return {a.x*s,a.y*s,a.z*s}; }
inline Vec3 operator*(float s, Vec3 a) { return a*s; }
inline Vec3 operator/(Vec3 a, float s) { return a*(1.0f/s); }
inline Vec3 operator*(Vec3 a, Vec3 b) { return {a.x*b.x,a.y*b.y,a.z*b.z}; }
inline Vec3& operator+=(Vec3& a, Vec3 b) { a=a+b; return a; }
inline float dot(Vec3 a, Vec3 b) { return a.x*b.x+a.y*b.y+a.z*b.z; }
inline Vec3 cross(Vec3 a, Vec3 b) {
    return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};
}
inline float length(Vec3 a) { return std::sqrt(dot(a,a)); }
inline Vec3 normalize(Vec3 a) { float l=length(a); return l>1e-7f ? a/l : Vec3(0,1,0); }
inline Vec3 mix(Vec3 a, Vec3 b, float t) { return a*(1-t)+b*t; }
struct Mat4 {
    // m[column*4 + row]; directly uploadable with glUniformMatrix4fv(..., GL_FALSE,...).
    float m[16]{};
    static Mat4 identity() { Mat4 r; r.m[0]=r.m[5]=r.m[10]=r.m[15]=1; return r; }
    const float* data() const { return m; }
};
inline Mat4 operator*(const Mat4& a,const Mat4& b) {
    Mat4 r;
    for(int c=0;c<4;++c) for(int row=0;row<4;++row)
        for(int k=0;k<4;++k) r.m[c*4+row]+=a.m[k*4+row]*b.m[c*4+k];
    return r;
}
inline Vec4 operator*(const Mat4& a,Vec4 b) {
    return {a.m[0]*b.x+a.m[4]*b.y+a.m[8]*b.z+a.m[12]*b.w,
            a.m[1]*b.x+a.m[5]*b.y+a.m[9]*b.z+a.m[13]*b.w,
            a.m[2]*b.x+a.m[6]*b.y+a.m[10]*b.z+a.m[14]*b.w,
            a.m[3]*b.x+a.m[7]*b.y+a.m[11]*b.z+a.m[15]*b.w};
}
inline Vec3 transformPoint(const Mat4& a,Vec3 p) {
    Vec4 r=a*Vec4{p.x,p.y,p.z,1}; return {r.x,r.y,r.z};
}
inline Mat4 translate(Vec3 p) {
    Mat4 r=Mat4::identity(); r.m[12]=p.x;r.m[13]=p.y;r.m[14]=p.z; return r;
}
inline Mat4 scale(Vec3 s) {
    Mat4 r=Mat4::identity(); r.m[0]=s.x;r.m[5]=s.y;r.m[10]=s.z; return r;
}
inline Mat4 rotateX(float a) {
    Mat4 r=Mat4::identity(); float c=std::cos(a),s=std::sin(a);
    r.m[5]=c;r.m[9]=-s;r.m[6]=s;r.m[10]=c;return r;
}
inline Mat4 rotateY(float a) {
    Mat4 r=Mat4::identity(); float c=std::cos(a),s=std::sin(a);
    r.m[0]=c;r.m[8]=s;r.m[2]=-s;r.m[10]=c;return r;
}
inline Mat4 rotateZ(float a) {
    Mat4 r=Mat4::identity(); float c=std::cos(a),s=std::sin(a);
    r.m[0]=c;r.m[4]=-s;r.m[1]=s;r.m[5]=c;return r;
}
inline Mat4 compose(Vec3 p,Vec3 angles,Vec3 s) {
    // With column vectors the rightmost operation happens first: S -> R -> T.
    return translate(p)*rotateZ(angles.z)*rotateY(angles.y)*rotateX(angles.x)*scale(s);
}
inline Mat4 perspective(float fov,float aspect,float nearZ,float farZ) {
    Mat4 r;float f=1.0f/std::tan(fov*0.5f);
    r.m[0]=f/aspect;r.m[5]=f;r.m[10]=(farZ+nearZ)/(nearZ-farZ);
    r.m[11]=-1;r.m[14]=2*farZ*nearZ/(nearZ-farZ);return r;
}
inline Mat4 lookAt(Vec3 eye,Vec3 target,Vec3 worldUp={0,1,0}) {
    Vec3 f=normalize(target-eye),s=normalize(cross(f,worldUp)),u=cross(s,f);
    Mat4 r=Mat4::identity();
    r.m[0]=s.x;r.m[4]=s.y;r.m[8]=s.z;
    r.m[1]=u.x;r.m[5]=u.y;r.m[9]=u.z;
    r.m[2]=-f.x;r.m[6]=-f.y;r.m[10]=-f.z;
    r.m[12]=-dot(s,eye);r.m[13]=-dot(u,eye);r.m[14]=dot(f,eye);return r;
}
} // namespace reef

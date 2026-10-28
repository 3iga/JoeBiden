#include "Math.hpp"
#include "Procedural.hpp"
#include <iostream>
#include <stdexcept>
using namespace reef;
void require(bool value,const char* message) { if(!value)throw std::runtime_error(message); }
bool near(float a,float b) { return std::abs(a-b)<.0001f; }
int main() {
    try {
        Vec3 p=transformPoint(compose({3,-1,2},{0,0,Pi/2},{2,3,4}),{1,0,0});
        require(near(p.x,3)&&near(p.y,1)&&near(p.z,2),"S -> R -> T order");
        Vec4 eye=lookAt({0,0,5},{0,0,0})*Vec4{0,0,0,1};
        require(near(eye.z,-5),"view transform");
        Mat4 projection=perspective(radians(60),16.0f/9,.1f,100);
        Vec4 nearPlane=projection*Vec4{0,0,-.1f,1},farPlane=projection*Vec4{0,0,-100,1};
        require(near(nearPlane.z/nearPlane.w,-1)&&near(farPlane.z/farPlane.w,1),"OpenGL depth convention");
        Vec3 parentChild=transformPoint(translate({1,0,0})*translate({0,2,0}),{0,0,0});
        require(near(parentChild.x,1)&&near(parentChild.y,2),"scene graph composition");
        auto a=scatter(1337,76,2.2f),b=scatter(1337,76,2.2f),c=scatter(1338,76,2.2f);
        require(a.size()==76&&a.size()==b.size(),"target density");
        bool different=false;
        for(std::size_t i=0;i<a.size();++i) {
            require(a[i].x==b[i].x&&a[i].y==b[i].y,"seed reproducibility");
            if(a[i].x!=c[i].x||a[i].y!=c[i].y)different=true;
            require(!reservedSpace(a[i].x,a[i].y),"reserved dive lane");
            for(std::size_t j=i+1;j<a.size();++j) {
                float dx=a[i].x-a[j].x,dz=a[i].y-a[j].y;
                require(dx*dx+dz*dz>=2.2f*2.2f-.00001f,"minimum seed spacing");
            }
        }
        require(different,"different seeds produce different scenes");
        require(scatter(4,0,2).empty(),"empty scatter");
        require(scatter(1337,128,1.75f).size()==128,"high density");
        std::cout<<"PASS: matrix order, view, projection, hierarchy, seed repeatability, spacing, dive lane, densities.\n";
        return 0;
    } catch(const std::exception& e) {std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
}

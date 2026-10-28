#include "Geometry.hpp"
#include "Procedural.hpp"
#include <functional>

namespace reef {
static void triangle(MeshData& m,unsigned a,unsigned b,unsigned c) {
    m.indices.insert(m.indices.end(),{a,b,c});
}
static void append(MeshData& target,const MeshData& source,Vec3 pos,Vec3 size) {
    unsigned base=static_cast<unsigned>(target.vertices.size());
    for(Vertex v:source.vertices) {
        v.position=v.position*size+pos;
        v.normal=normalize(Vec3(v.normal.x/size.x,v.normal.y/size.y,v.normal.z/size.z));
        target.vertices.push_back(v);
    }
    for(unsigned i:source.indices)target.indices.push_back(base+i);
}
MeshData makeSphere(int slices,int rings) {
    MeshData m;
    for(int j=0;j<=rings;++j) {
        float v=float(j)/rings,phi=Pi*v;
        for(int i=0;i<=slices;++i) {
            float u=float(i)/slices,theta=2*Pi*u;
            Vec3 p(std::sin(phi)*std::cos(theta),std::cos(phi),std::sin(phi)*std::sin(theta));
            m.vertices.push_back({p,p,{u,v}});
        }
    }
    for(int j=0;j<rings;++j)for(int i=0;i<slices;++i) {
        unsigned a=unsigned(j*(slices+1)+i),b=a+unsigned(slices+1);
        triangle(m,a,a+1,b);triangle(m,a+1,b+1,b);
    }
    return m;
}
MeshData makeSeabed() {
    MeshData m;constexpr int S=96;constexpr float half=36;
    for(int j=0;j<=S;++j)for(int i=0;i<=S;++i) {
        float x=-half+2*half*float(i)/S,z=-half+2*half*float(j)/S;
        constexpr float e=.05f;
        float dx=(seabedHeight(x+e,z)-seabedHeight(x-e,z))/(2*e);
        float dz=(seabedHeight(x,z+e)-seabedHeight(x,z-e))/(2*e);
        m.vertices.push_back({{x,seabedHeight(x,z),z},normalize(Vec3(-dx,1,-dz)),{float(i)/S,float(j)/S}});
    }
    for(int j=0;j<S;++j)for(int i=0;i<S;++i) {
        unsigned a=unsigned(j*(S+1)+i),b=a+S+1;
        triangle(m,a,b,a+1);triangle(m,a+1,b,b+1);
    }
    return m;
}
static void cylinder(MeshData& m,Vec3 a,Vec3 b,float ra,float rb,int sides=9) {
    Vec3 axis=normalize(b-a),helper=std::abs(axis.y)>.9f?Vec3(1,0,0):Vec3(0,1,0);
    Vec3 right=normalize(cross(axis,helper)),front=cross(axis,right);
    unsigned base=static_cast<unsigned>(m.vertices.size());float h=length(b-a);
    for(int row=0;row<2;++row)for(int i=0;i<=sides;++i) {
        float u=float(i)/sides,angle=u*2*Pi;Vec3 radial=right*std::cos(angle)+front*std::sin(angle);
        Vec3 p=(row?b:a)+radial*(row?rb:ra);
        Vec3 n=normalize(radial+axis*((ra-rb)/std::max(h,.001f)));
        m.vertices.push_back({p,n,{u,float(row)}});
    }
    for(int i=0;i<sides;++i) {
        unsigned a0=base+unsigned(i),b0=a0+unsigned(sides+1);
        triangle(m,a0,a0+1,b0);triangle(m,a0+1,b0+1,b0);
    }
    // A low-poly rounded tip closes each branch without loading a model.
    unsigned center=static_cast<unsigned>(m.vertices.size());
    m.vertices.push_back({b+axis*rb*.15f,axis,{.5f,.5f}});
    for(int i=0;i<sides;++i)triangle(m,center,base+unsigned(sides+1+i),base+unsigned(sides+2+i));
}
MeshData makeBranchCoral() {
    MeshData m;Random rng(117);MeshData bulb=makeSphere(8,5);
    std::function<void(Vec3,Vec3,float,float,int)> branch;
    branch=[&](Vec3 start,Vec3 direction,float len,float radius,int depth) {
        Vec3 end=start+normalize(direction)*len;
        cylinder(m,start,end,radius,radius*.63f);
        append(m,bulb,end,Vec3(radius*.68f));
        if(depth==0)return;
        int count=depth==2?3:2;
        for(int i=0;i<count;++i) {
            float a=rng.range(0,2*Pi);
            Vec3 d=normalize(direction*.56f+Vec3(std::cos(a)*.72f,.55f,std::sin(a)*.72f));
            branch(end,d,len*rng.range(.59f,.78f),radius*.60f,depth-1);
        }
    };
    cylinder(m,{0,0,0},{0,.38f,0},.21f,.17f);
    for(int i=0;i<6;++i) {
        float a=2*Pi*float(i)/6;
        branch({0,.24f,0},{std::cos(a)*.65f,.95f,std::sin(a)*.65f},rng.range(.65f,.95f),.13f,2);
    }
    branch({0,.25f,0},{0,1,0},.95f,.14f,2);
    return m;
}
MeshData makeTubeCoral() {
    MeshData m;Random rng(93);constexpr int S=18;
    for(int tube=0;tube<9;++tube) {
        float a=float(tube)*2.399963f,r=std::sqrt(float(tube))*.25f;
        Vec3 center(std::cos(a)*r,0,std::sin(a)*r);
        float height=rng.range(.65f,1.55f),radius=rng.range(.14f,.25f);
        unsigned base=static_cast<unsigned>(m.vertices.size());
        // Four rings: outside bottom, outside lip, inside lip, dark inside bottom.
        for(int ring=0;ring<4;++ring)for(int i=0;i<=S;++i) {
            float u=float(i)/S,theta=u*2*Pi;
            float rr=(ring==0?radius*.70f:(ring==1?radius:(ring==2?radius*.67f:radius*.42f)));
            float y=(ring==0?0:(ring<3?height:height*.35f));
            Vec3 p=center+Vec3(rr*std::cos(theta)+.1f*y,y,rr*std::sin(theta));
            Vec3 n=ring<2?normalize(Vec3(std::cos(theta),-.10f,std::sin(theta))):normalize(Vec3(-std::cos(theta),.08f,-std::sin(theta)));
            m.vertices.push_back({p,n,{u,ring==3?.2f:float(ring)*.45f}});
        }
        for(int ring=0;ring<3;++ring)for(int i=0;i<S;++i) {
            unsigned p=base+unsigned(ring*(S+1)+i),q=p+S+1;
            triangle(m,p,q,p+1);triangle(m,p+1,q,q+1);
        }
        unsigned c=static_cast<unsigned>(m.vertices.size());
        m.vertices.push_back({center+Vec3(.035f*height,height*.35f,0),{0,1,0},{.5f,.5f}});
        for(int i=0;i<S;++i)triangle(m,c,base+3*(S+1)+unsigned(i+1),base+3*(S+1)+unsigned(i));
    }
    return m;
}
MeshData makePlateCoral() {
    MeshData m;MeshData orb=makeSphere(32,12);
    for(int level=0;level<5;++level) {
        float a=level*2.35f,r=.95f-level*.11f;
        MeshData plate=orb;
        for(auto& v:plate.vertices) {
            float theta=std::atan2(v.position.z,v.position.x);
            float wave=1+.10f*std::sin(8*theta+level)*std::sin(std::acos(clamp(v.position.y,-1,1)));
            v.position.x*=wave;v.position.z*=wave;
        }
        append(m,plate,{.16f*std::sin(a),.3f+level*.27f,.18f*std::cos(a)},{r,.10f,r});
    }
    cylinder(m,{0,0,0},{0,1.25f,0},.15f,.075f);
    return m;
}
MeshData makeRock() {
    MeshData m=makeSphere(18,12);
    for(auto& v:m.vertices) {
        Vec3 p=v.position;
        float k=1+.13f*std::sin(6*p.x+3*p.y)+.10f*std::sin(5*p.z-4*p.y)+.06f*std::sin(11*p.x+7*p.z);
        v.position=p*k;v.normal=Vec3(0);
    }
    // Accumulated face normals give the deformed mesh, rather than the original sphere, its lighting.
    for(std::size_t i=0;i<m.indices.size();i+=3) {
        Vertex& a=m.vertices[m.indices[i]];Vertex& b=m.vertices[m.indices[i+1]];Vertex& c=m.vertices[m.indices[i+2]];
        Vec3 n=cross(b.position-a.position,c.position-a.position);
        a.normal+=n;b.normal+=n;c.normal+=n;
    }
    for(auto& v:m.vertices)v.normal=normalize(v.normal);
    return m;
}
MeshData makeKelp() {
    MeshData m;Random rng(531);
    for(int leaf=0;leaf<7;++leaf) {
        float height=rng.range(1.5f,3.4f),angle=rng.range(0,2*Pi),width=rng.range(.16f,.32f);
        Vec3 lateral(std::cos(angle),0,std::sin(angle));
        unsigned base=static_cast<unsigned>(m.vertices.size());
        for(int j=0;j<=12;++j) {
            float t=float(j)/12;
            Vec3 center=lateral*(.22f*std::sin(t*4+leaf)*t)+Vec3(.13f*std::sin(leaf*3.1f),t*height,.13f*std::cos(leaf*3.1f));
            float w=width*std::sin(Pi*std::pow(t,.65f))+.006f;
            Vec3 n=normalize(cross(lateral,Vec3(0,1,0)));
            m.vertices.push_back({center-lateral*w,n,{0,t}});m.vertices.push_back({center+lateral*w,n,{1,t}});
        }
        for(int j=0;j<12;++j) { unsigned a=base+unsigned(j*2);triangle(m,a,a+1,a+2);triangle(m,a+1,a+3,a+2); }
    }
    return m;
}
MeshData makeTail() {
    MeshData m;
    // Tail is local to its pivot, which sits behind the fish body.
    m.vertices={{{0,0,0},{0,0,1},{.5f,.5f}},{{-.36f,.30f,0},{0,0,1},{0,1}},{{-.36f,-.30f,0},{0,0,1},{0,0}}};
    m.indices={0,1,2};return m;
}
MeshData makeFin() {
    MeshData m;
    m.vertices={{{-.30f,0,0},{0,0,1},{0,0}},{{.20f,0,0},{0,0,1},{1,0}},{{-.12f,.28f,0},{0,0,1},{.5f,1}}};
    m.indices={0,1,2};return m;
}
MeshData makeRing() {
    MeshData m;constexpr int S=80;
    for(int i=0;i<=S;++i) {
        float a=2*Pi*float(i)/S;
        for(int j=0;j<2;++j) {float r=j?1.45f:1.40f;m.vertices.push_back({{r*std::cos(a),.015f,r*std::sin(a)},{0,1,0},{float(i)/S,float(j)}});}
    }
    for(int i=0;i<S;++i){unsigned a=unsigned(i*2);triangle(m,a,a+2,a+1);triangle(m,a+1,a+2,a+3);}return m;
}
} // namespace reef

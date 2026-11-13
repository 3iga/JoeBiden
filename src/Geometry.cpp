#include "Geometry.hpp"
#include "Procedural.hpp"

static void triangle(MeshData& m, unsigned a, unsigned b, unsigned c) {
    m.indices.insert(m.indices.end(), {a, b, c});
}
static void append(MeshData& target, const MeshData& source, Vec3 pos, Vec3 size) {
    unsigned base=static_cast<unsigned>(target.vertices.size());
    for (Vertex v:source.vertices) {
        v.position=v.position*size+pos;
        v.normal=safeNormalize(Vec3(v.normal.x/size.x, v.normal.y/size.y, v.normal.z/size.z));
        target.vertices.push_back(v);
    }
    for (unsigned i:source.indices)target.indices.push_back(base+i);
}
static void appendStoneBlock(MeshData& mesh, Vec3 center, Vec3 halfSize) {
    const Vec3 normals[6]={{1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}, {0, 0, 1}, {0, 0, -1}};
    const Vec3 faces[6][4]={
        {{halfSize.x, -halfSize.y, -halfSize.z}, {halfSize.x, halfSize.y, -halfSize.z}, {halfSize.x, halfSize.y, halfSize.z}, {halfSize.x, -halfSize.y, halfSize.z}},
        {{-halfSize.x, -halfSize.y, halfSize.z}, {-halfSize.x, halfSize.y, halfSize.z}, {-halfSize.x, halfSize.y, -halfSize.z}, {-halfSize.x, -halfSize.y, -halfSize.z}},
        {{-halfSize.x, halfSize.y, -halfSize.z}, {-halfSize.x, halfSize.y, halfSize.z}, {halfSize.x, halfSize.y, halfSize.z}, {halfSize.x, halfSize.y, -halfSize.z}},
        {{-halfSize.x, -halfSize.y, halfSize.z}, {-halfSize.x, -halfSize.y, -halfSize.z}, {halfSize.x, -halfSize.y, -halfSize.z}, {halfSize.x, -halfSize.y, halfSize.z}},
        {{halfSize.x, -halfSize.y, halfSize.z}, {halfSize.x, halfSize.y, halfSize.z}, {-halfSize.x, halfSize.y, halfSize.z}, {-halfSize.x, -halfSize.y, halfSize.z}},
        {{-halfSize.x, -halfSize.y, -halfSize.z}, {-halfSize.x, halfSize.y, -halfSize.z}, {halfSize.x, halfSize.y, -halfSize.z}, {halfSize.x, -halfSize.y, -halfSize.z}}
    };
    const float uv[4][2]={{0, 0}, {0, 1}, {1, 1}, {1, 0}};
    for (unsigned face=0; face<6; ++face) {
        const unsigned base=static_cast<unsigned>(mesh.vertices.size());
        for (unsigned corner=0; corner<4; ++corner)
            mesh.vertices.push_back({center+faces[face][corner], normals[face], {uv[corner][0], uv[corner][1]}});
        triangle(mesh, base, base+1, base+2);
        triangle(mesh, base, base+2, base+3);
    }
}
MeshData makeSphere(int slices, int rings) {
    MeshData m;
    for (int j=0; j<=rings; ++j) {
        float v=float(j)/rings, phi=Pi*v;
        for (int i=0; i<=slices; ++i) {
            float u=float(i)/slices, theta=2*Pi*u;
            Vec3 p(std::sin(phi)*std::cos(theta), std::cos(phi), std::sin(phi)*std::sin(theta));
            m.vertices.push_back({p, p, {u, v}});
        }
    }
    for (int j=0; j<rings; ++j)for (int i=0; i<slices; ++i) {
        unsigned a=unsigned(j*(slices+1)+i), b=a+unsigned(slices+1);
        triangle(m, a, a+1, b);
        triangle(m, a+1, b+1, b);
    }
    return m;
}
static void cylinder(MeshData& m, Vec3 a, Vec3 b, float ra, float rb, int sides=9) {
    Vec3 axis=safeNormalize(b-a), helper=std::abs(axis.y)>.9f?Vec3(1, 0, 0):Vec3(0, 1, 0);
    Vec3 right=safeNormalize(cross(axis, helper)), front=cross(axis, right);
    unsigned base=static_cast<unsigned>(m.vertices.size());
    float h=length(b-a);
    for (int row=0; row<2; ++row)for (int i=0; i<=sides; ++i) {
        float u=float(i)/sides, angle=u*2*Pi;
        Vec3 radial=right*std::cos(angle)+front*std::sin(angle);
        Vec3 p=(row?b:a)+radial*(row?rb:ra);
        Vec3 n=safeNormalize(radial+axis*((ra-rb)/std::max(h, .001f)));
        m.vertices.push_back({p, n, {u, float(row)}});
    }
    for (int i=0; i<sides; ++i) {
        unsigned a0=base+unsigned(i), b0=a0+unsigned(sides+1);
        triangle(m, a0, a0+1, b0);
        triangle(m, a0+1, b0+1, b0);
    }
    // A low-poly rounded tip closes each branch without loading a model.
    unsigned center=static_cast<unsigned>(m.vertices.size());
    m.vertices.push_back({b+axis*rb*.15f, axis, {.5f, .5f}});
    for (int i=0; i<sides; ++i)triangle(m, center, base+unsigned(sides+1+i), base+unsigned(sides+2+i));
}
MeshData makeTubeCoral(std::uint32_t seed) {
    MeshData m;
    Random rng(seed);
    constexpr int S=18;
    for (int tube=0; tube<9; ++tube) {
        float a=float(tube)*2.399963f, r=std::sqrt(float(tube))*.25f;
        Vec3 center(std::cos(a)*r, 0, std::sin(a)*r);
        float height=rng.range(.65f, 1.55f), radius=rng.range(.14f, .25f);
        unsigned base=static_cast<unsigned>(m.vertices.size());
        // Four rings: outside bottom, outside lip, inside lip, dark inside bottom.
        for (int ring=0; ring<4; ++ring)for (int i=0; i<=S; ++i) {
            float u=float(i)/S, theta=u*2*Pi;
            float rr=(ring==0?radius*.70f:(ring==1?radius:(ring==2?radius*.67f:radius*.42f)));
            float y=(ring==0?0:(ring<3?height:height*.35f));
            Vec3 p=center+Vec3(rr*std::cos(theta)+.1f*y, y, rr*std::sin(theta));
            Vec3 n=ring<2?safeNormalize(Vec3(std::cos(theta), -.10f, std::sin(theta))):safeNormalize(Vec3(-std::cos(theta), .08f, -std::sin(theta)));
            m.vertices.push_back({p, n, {u, ring==3?.2f:float(ring)*.45f}});
        }
        for (int ring=0; ring<3; ++ring)for (int i=0; i<S; ++i) {
            unsigned p=base+unsigned(ring*(S+1)+i), q=p+S+1;
            triangle(m, p, q, p+1);
            triangle(m, p+1, q, q+1);
        }
        unsigned c=static_cast<unsigned>(m.vertices.size());
        m.vertices.push_back({center+Vec3(.035f*height, height*.35f, 0), {0, 1, 0}, {.5f, .5f}});
        for (int i=0; i<S; ++i)triangle(m, c, base+3*(S+1)+unsigned(i+1), base+3*(S+1)+unsigned(i));
    }
    return m;
}
MeshData makeRock() {
    MeshData m=makeSphere(18, 12);
    for (auto& v:m.vertices) {
        Vec3 p=v.position;
        float k=1+.13f*std::sin(6*p.x+3*p.y)+.10f*std::sin(5*p.z-4*p.y)+.06f*std::sin(11*p.x+7*p.z);
        v.position=p*k;
        v.normal=Vec3(0);
    }
    // Accumulated face normals give the deformed mesh, rather than the original sphere, its lighting.
    for (std::size_t i=0; i<m.indices.size(); i+=3) {
        Vertex& a=m.vertices[m.indices[i]];
        Vertex& b=m.vertices[m.indices[i+1]];
        Vertex& c=m.vertices[m.indices[i+2]];
        Vec3 n=cross(b.position-a.position, c.position-a.position);
        a.normal+=n;
        b.normal+=n;
        c.normal+=n;
    }
    for (auto& v:m.vertices)v.normal=safeNormalize(v.normal);
    return m;
}
MeshData makeUnderwaterStructure() {
    MeshData mesh;
    // A broken stone gateway assembled from broad, individually eroded blocks.
    const float sides[]={-1.0f, 1.0f};
    for (float side:sides) {
        const float x=side*2.15f;
        appendStoneBlock(mesh, {x, .28f, 0}, {.82f, .28f, .82f});
        appendStoneBlock(mesh, {x-side*.08f, .94f, 0}, {.56f, .43f, .60f});
        appendStoneBlock(mesh, {x+side*.06f, 1.82f, .03f}, {.50f, .45f, .56f});
        appendStoneBlock(mesh, {x-side*.05f, 2.68f, 0}, {.66f, .36f, .68f});
    }
    appendStoneBlock(mesh, {-1.12f, 3.27f, 0}, {1.13f, .34f, .70f});
    appendStoneBlock(mesh, {1.75f, 3.08f, .02f}, {.74f, .32f, .67f});
    // Fallen masonry makes the ruin look collapsed and gives it a wider seabed footprint.
    appendStoneBlock(mesh, {.70f, .36f, 1.02f}, {1.08f, .34f, .54f});
    appendStoneBlock(mesh, {-2.45f, .34f, -1.12f}, {.71f, .32f, .62f});
    appendStoneBlock(mesh, {2.52f, .30f, -1.18f}, {.60f, .28f, .54f});
    return mesh;
}
static void appendBrokenHull(MeshData& mesh, float length, float beam, int sections, bool splitMiddle) {
    for (int i=0; i<sections; ++i) {
        if (splitMiddle&&i==sections/2)continue;
        const float t=(float(i)+.5f)/float(sections)*2.0f-1.0f;
        const float taper=.42f+.58f*(1.0f-std::pow(std::abs(t), 1.5f));
        const float x=-length*.5f+(float(i)+.5f)*length/float(sections);
        const float halfLength=length/float(sections)*.54f;
        appendStoneBlock(mesh, {x, .50f, 0}, {halfLength, .43f, beam*taper});
    }
}
static MeshData makeCargoWreck() {
    MeshData mesh;
    appendBrokenHull(mesh, 17.0f, 1.75f, 7, true);
    appendStoneBlock(mesh, {-2.9f, 1.13f, 0}, {2.35f, .28f, 1.43f});
    for (int i=0; i<4; ++i) {
        const float x=-4.7f+float(i)*2.05f;
        appendStoneBlock(mesh, {x, 1.73f, .02f}, {.76f, .32f, .59f});
    }
    appendStoneBlock(mesh, {-5.3f, 2.15f, 0}, {.65f, .50f, .60f});
    cylinder(mesh, {-5.3f, 2.5f, 0}, {-5.3f, 4.7f, 0}, .10f, .035f, 7);
    appendStoneBlock(mesh, {1.7f, .42f, 2.1f}, {1.05f, .26f, .36f});
    appendStoneBlock(mesh, {5.3f, .32f, -1.8f}, {.72f, .22f, .48f});
    return mesh;
}
static MeshData makeLinerWreck() {
    MeshData mesh;
    appendBrokenHull(mesh, 23.0f, 2.65f, 9, true);
    appendStoneBlock(mesh, {-2.0f, 1.02f, 0}, {7.7f, .23f, 2.18f});
    for (int deck=0; deck<3; ++deck) {
        const float y=1.42f+float(deck)*.62f;
        appendStoneBlock(mesh, {-2.7f+float(deck)*.35f, y, 0}, {5.0f-float(deck)*.7f, .23f, 1.65f-float(deck)*.17f});
    }
    for (int i=0; i<4; ++i) {
        const float x=-7.4f+float(i)*3.4f;
        cylinder(mesh, {x, 1.9f, 0}, {x, 3.35f, 0}, .34f, .27f, 8);
    }
    cylinder(mesh, {7.6f, .95f, 0}, {9.2f, 2.9f, 0}, .20f, .04f, 7);
    return mesh;
}
static MeshData makeCarrierWreck() {
    MeshData mesh;
    appendBrokenHull(mesh, 22.0f, 2.2f, 8, false);
    appendStoneBlock(mesh, {0, 1.06f, 0}, {10.3f, .24f, 3.35f});
    appendStoneBlock(mesh, {-3.0f, 1.55f, .5f}, {2.2f, .28f, 1.50f});
    appendStoneBlock(mesh, {3.2f, 1.72f, -1.5f}, {.85f, .75f, .72f});
    cylinder(mesh, {3.2f, 2.35f, -1.5f}, {3.2f, 3.6f, -1.5f}, .10f, .07f, 7);
    appendStoneBlock(mesh, {-6.3f, .52f, 2.8f}, {1.1f, .15f, .35f});
    appendStoneBlock(mesh, {0, .48f, -2.95f}, {2.1f, .16f, .38f});
    return mesh;
}
static MeshData makeAircraftWreck() {
    MeshData mesh;
    appendStoneBlock(mesh, {0, .50f, 0}, {3.3f, .28f, .30f});
    appendStoneBlock(mesh, {-.35f, .46f, 0}, {.90f, .13f, 4.9f});
    appendStoneBlock(mesh, {-2.55f, .48f, 0}, {.65f, .12f, 1.55f});
    appendStoneBlock(mesh, {-2.95f, .45f, 0}, {.55f, .12f, .20f});
    appendStoneBlock(mesh, {2.0f, .43f, .23f}, {1.2f, .12f, .22f});
    appendStoneBlock(mesh, {1.9f, .42f, -1.8f}, {.68f, .10f, .20f});
    cylinder(mesh, {3.15f, .5f, 0}, {3.55f, .5f, 0}, .06f, .02f, 7);
    return mesh;
}
static MeshData makeSubmarineWreck() {
    MeshData mesh;
    cylinder(mesh, {-6.0f, .75f, 0}, {5.5f, .75f, 0}, .83f, .55f, 14);
    appendStoneBlock(mesh, {.3f, 1.48f, 0}, {1.0f, .55f, .56f});
    cylinder(mesh, {.3f, 1.95f, 0}, {.3f, 2.65f, 0}, .08f, .04f, 7);
    appendStoneBlock(mesh, {-5.8f, .72f, 0}, {.52f, .16f, 1.25f});
    appendStoneBlock(mesh, {5.05f, .78f, 0}, {.45f, .15f, 1.05f});
    return mesh;
}
static MeshData makeFishingBoatWreck() {
    MeshData mesh;
    appendBrokenHull(mesh, 8.5f, 1.12f, 5, false);
    appendStoneBlock(mesh, {-1.8f, 1.02f, 0}, {1.65f, .48f, .87f});
    appendStoneBlock(mesh, {1.8f, .91f, 0}, {1.4f, .26f, .90f});
    cylinder(mesh, {-1.9f, 1.4f, 0}, {-1.9f, 3.4f, 0}, .07f, .02f, 7);
    appendStoneBlock(mesh, {3.2f, .38f, .95f}, {.78f, .18f, .22f});
    return mesh;
}
MeshData makeStructure(int variant) {
    switch (variant) {
    case 1:return makeCargoWreck();
    case 2:return makeLinerWreck();
    case 3:return makeCarrierWreck();
    case 4:return makeAircraftWreck();
    case 5:return makeCarrierWreck();
    case 6:return makeSubmarineWreck();
    case 7:return makeFishingBoatWreck();
    default:return makeUnderwaterStructure();
    }
}
MeshData makeDebris(int variant) {
    MeshData mesh;
    switch (variant) {
    case 0:
        cylinder(mesh, {0, 0, 0}, {0, .54f, 0}, .22f, .22f, 10);
        cylinder(mesh, {0, .54f, 0}, {0, .57f, 0}, .18f, .18f, 10);
        break;
    case 1:
        cylinder(mesh, {0, 0, 0}, {0, .40f, 0}, .14f, .13f, 9);
        cylinder(mesh, {0, .38f, 0}, {0, .62f, 0}, .08f, .06f, 8);
        break;
    case 2:
        appendStoneBlock(mesh, {0, .30f, 0}, {.38f, .30f, .38f});
        appendStoneBlock(mesh, {0, .30f, .39f}, {.39f, .035f, .035f});
        appendStoneBlock(mesh, {0, .30f, -.39f}, {.39f, .035f, .035f});
        break;
    case 3: {
        MeshData ring=makeRing();
        append(mesh, ring, {0, 0, 0}, Vec3(.56f));
        break;
    }
    default:
        for (int i=0; i<9; ++i) {
            Vec3 p{-1.0f, .04f+float(i%3)*.03f, -1.0f+float(i)*.25f};
            Vec3 q{1.0f, .04f+float((i+1)%3)*.03f, 1.0f-float(i)*.25f};
            cylinder(mesh, p, q, .018f, .018f, 4);
        }
        for (int i=0; i<9; ++i) {
            const float z=-1.0f+float(i)*.25f;
            cylinder(mesh, {-1.0f, .08f, z}, {1.0f, .08f, z}, .014f, .014f, 4);
        }
        break;
    }
    return mesh;
}
MeshData makeKelp() {
    MeshData m;
    m.twoSided = true;
    Random rng(531);
    for (int leaf=0; leaf<7; ++leaf) {
        float height=rng.range(1.5f, 3.4f), angle=rng.range(0, 2*Pi), width=rng.range(.16f, .32f);
        Vec3 lateral(std::cos(angle), 0, std::sin(angle));
        unsigned base=static_cast<unsigned>(m.vertices.size());
        for (int j=0; j<=12; ++j) {
            float t=float(j)/12;
            Vec3 center=lateral*(.22f*std::sin(t*4+leaf)*t)+Vec3(.13f*std::sin(leaf*3.1f), t*height, .13f*std::cos(leaf*3.1f));
            float w=width*std::sin(Pi*std::pow(t, .65f))+.006f;
            Vec3 n=safeNormalize(cross(lateral, Vec3(0, 1, 0)));
            m.vertices.push_back({center-lateral*w, n, {0, t}});
            m.vertices.push_back({center+lateral*w, n, {1, t}});
        }
        for (int j=0; j<12; ++j) {
            unsigned a=base+unsigned(j*2);
            triangle(m, a, a+1, a+2);
            triangle(m, a+1, a+3, a+2);
        }
    }
    return m;
}
MeshData makeSeagrass() {
    MeshData m;
    m.twoSided=true;
    constexpr int Steps=7, Blades=22;
    for (int blade=0; blade<Blades; ++blade) {
        const float angle=float(blade)*2.399963f;
        const Vec3 forward{std::cos(angle), 0, std::sin(angle)};
        const Vec3 side{-forward.z, 0, forward.x};
        const float height=.38f+.62f*float((blade*7)%Blades)/float(Blades);
        const float width=.028f+.018f*float(blade%4)/3.0f;
        const unsigned base=static_cast<unsigned>(m.vertices.size());
        for (int step=0; step<=Steps; ++step) {
            const float t=float(step)/Steps;
            const Vec3 center=forward*(.34f*t*t)+Vec3(.09f*std::sin(t*Pi+angle), height*t, .09f*std::cos(t*Pi+angle));
            const float w=width*(1.0f-t)+.003f;
            const Vec3 normal=safeNormalize(cross(forward, Vec3(0, 1, 0)));
            m.vertices.push_back({center-side*w, normal, {0, t}});
            m.vertices.push_back({center+side*w, normal, {1, t}});
        }
        for (int step=0; step<Steps; ++step) {
            const unsigned a=base+unsigned(step*2);
            triangle(m, a, a+1, a+2);
            triangle(m, a+1, a+3, a+2);
        }
    }
    return m;
}
MeshData makeTail() {
    MeshData m;
    m.twoSided = true;
    // Tail is local to its pivot, which sits behind the fish body.
    m.vertices={{{0, 0, 0}, {0, 0, 1}, {.5f, .5f}}, {{-.36f, .30f, 0}, {0, 0, 1}, {0, 1}}, {{-.36f, -.30f, 0}, {0, 0, 1}, {0, 0}}};
    m.indices={0, 1, 2};
    return m;
}
MeshData makeFin() {
    MeshData m;
    m.twoSided = true;
    m.vertices={{{-.30f, 0, 0}, {0, 0, 1}, {0, 0}}, {{.20f, 0, 0}, {0, 0, 1}, {1, 0}}, {{-.12f, .28f, 0}, {0, 0, 1}, {.5f, 1}}};
    m.indices={0, 1, 2};
    return m;
}
MeshData makeRing() {
    MeshData m;
    constexpr int S=80;
    for (int i=0; i<=S; ++i) {
        float a=2*Pi*float(i)/S;
        for (int j=0; j<2; ++j) {
            float r=j?1.45f:1.40f;
            m.vertices.push_back({{r*std::cos(a), .015f, r*std::sin(a)}, {0, 1, 0}, {float(i)/S, float(j)}});
        }
    }
    for (int i=0; i<S; ++i) {
        unsigned a=unsigned(i*2);
        triangle(m, a, a+2, a+1);
        triangle(m, a+1, a+2, a+3);
    }
    return m;
}

void recalculateNormals(MeshData& m) {
    for (auto& v:m.vertices)v.normal={0, 0, 0};
    for (std::size_t i=0; i+2<m.indices.size(); i+=3) {
        auto& a=m.vertices[m.indices[i]];
        auto& b=m.vertices[m.indices[i+1]];
        auto& c=m.vertices[m.indices[i+2]];
        Vec3 n=cross(b.position-a.position, c.position-a.position);
        a.normal+=n;
        b.normal+=n;
        c.normal+=n;
    }
    for (auto& v:m.vertices)v.normal=safeNormalize(v.normal);
}
MeshData makeTerrainTile(ChunkKey key, std::uint32_t seed) {
    MeshData m;
    constexpr int N=20;
    const float half=float(ChunkSize*.5);
    for (int z=0; z<=N; ++z)for (int x=0; x<=N; ++x) {
        float lx=-half+float(ChunkSize)*float(x)/N, lz=-half+float(ChunkSize)*float(z)/N;
        double wx=double(key.first)*ChunkSize+lx, wz=double(key.second)*ChunkSize+lz;
        const double e=.03;
        float dx=(terrainHeight(wx+e, wz, seed)-terrainHeight(wx-e, wz, seed))/float(2*e);
        float dz=(terrainHeight(wx, wz+e, seed)-terrainHeight(wx, wz-e, seed))/float(2*e);
        // Texture period is 3 world units. Integer chunk offsets give seamless wrapping.
        m.vertices.push_back({{lx, terrainHeight(wx, wz, seed), lz}, safeNormalize({-dx, 1, -dz}), {lx/3, lz/3}});
    }
    for (int z=0; z<N; ++z)for (int x=0; x<N; ++x) {
        unsigned a=unsigned(z*(N+1)+x), b=a+N+1;
        triangle(m, a, b, a+1);
        triangle(m, a+1, b, b+1);
    }
    return m;
}
MeshData makeGrowthCoral(std::uint32_t seed, bool fan) {
    // Space-colonization-style attraction points (Runions et al., 2007).
    // This is a lightweight graphics adaptation, NOT Joel Simon's genetic/differential-growth model.
    struct Segment {Vec3 p;
    int parent;
    };
    Random rng(seed);
    std::vector<Vec3> attraction;
    std::vector<Segment> tree;
    const float spread=rng.range(.9f, 1.25f), tall=rng.range(1.0f, 1.3f);
    for (int i=0; i<230; ) {
        Vec3 q{rng.range(-1, 1), rng.range(-1, 1), rng.range(-1, 1)};
        if (dot(q, q)>1)continue;
        attraction.push_back({q.x*spread, (q.y*.92f+1.40f)*tall, q.z*(fan?.19f:spread*.80f)});
        ++i;
    }
    tree.push_back({{0, 0, 0}, -1});
    tree.push_back({{0, .16f, 0}, 0});
    tree.push_back({{.02f, .33f, 0}, 1});
    const float step=.135f, influence=.82f, kill=.18f;
    for (int iteration=0; iteration<65&&tree.size()<380&&!attraction.empty(); ++iteration) {
        std::size_t count=tree.size();
        std::vector<Vec3> sums(count, Vec3(0));
        std::vector<int> hits(count, 0);
        std::vector<Vec3> remaining;
        for (Vec3 a:attraction) {
            int nearest=-1;
            float best=influence;
            bool consumed=false;
            for (std::size_t j=0; j<count; ++j) {
                float d=length(a-tree[j].p);
                if (d<kill) {
                    consumed=true;
                    break;
                }
                if (d<best) {
                    best=d;
                    nearest=int(j);
                }
            }
            if (consumed)continue;
            remaining.push_back(a);
            if (nearest>=0) {
                sums[std::size_t(nearest)]+=safeNormalize(a-tree[std::size_t(nearest)].p);
                ++hits[std::size_t(nearest)];
            }
        }
        attraction.swap(remaining);
        int grown=0;
        for (std::size_t j=0; j<count&&tree.size()<380; ++j)if (hits[j]) {
            Vec3 d=safeNormalize(sums[j]/float(hits[j])+Vec3(0, .12f, 0));
            Vec3 p=tree[j].p+d*step;
            bool duplicate=false;
            for (std::size_t k=0; k<tree.size(); ++k)if (k!=j&&length(tree[k].p-p)<step*.56f) {
                duplicate=true;
                break;
            }
            if (!duplicate) {
                tree.push_back({p, int(j)});
                ++grown;
            }
        }
        if (grown==0)break;
    }
    std::vector<int> leaves(tree.size(), 1), children(tree.size(), 0);
    for (std::size_t j=1; j<tree.size(); ++j)++children[std::size_t(tree[j].parent)];
    for (int j=int(tree.size())-1; j>0; --j)leaves[std::size_t(tree[std::size_t(j)].parent)]+=leaves[std::size_t(j)];
    MeshData mesh, tip=makeSphere(7, 4);
    auto radius=[&](std::size_t j) {
        return std::min(.17f, .016f+.0085f*std::sqrt(float(leaves[j])));
    };
    for (std::size_t j=1; j<tree.size(); ++j) {
        auto parent=std::size_t(tree[j].parent);
        float r=radius(j);
        cylinder(mesh, tree[parent].p, tree[j].p, radius(parent), r, 7);
        if (children[j]==0)append(mesh, tip, tree[j].p, Vec3(r*1.25f));
    }
    // Smooth-looking junctions use small generated spheres; bounded once at startup.
    for (std::size_t j=0; j<tree.size(); ++j)if (children[j]>1)append(mesh, tip, tree[j].p, Vec3(radius(j)*1.08f));
    return mesh;
}
MeshData makeRuffledCoral(std::uint32_t seed) {
    Random rng(seed);
    MeshData m;
    m.twoSided = true;
    constexpr int S=48, R=7;
    for (int plate=0; plate<4; ++plate) {
        float radius=1.1f-float(plate)*.16f, phase=rng.range(0, 2*Pi);
        unsigned base=static_cast<unsigned>(m.vertices.size());
        for (int ring=0; ring<=R; ++ring)for (int j=0; j<=S; ++j) {
            float f=float(ring)/R, a=float(j)*2*Pi/S;
            float r=radius*f*(1+.11f*std::sin(a*7+phase)*f+.035f*std::sin(a*15-phase));
            float y=.28f+plate*.36f+.24f*f*f+std::pow(f, 3.0f)*.10f*std::sin(a*8+phase);
            m.vertices.push_back({{r*std::cos(a), y, r*std::sin(a)}, {0, 1, 0}, {f*std::cos(a)*.5f+.5f, f*std::sin(a)*.5f+.5f}});
        }
        for (int ring=0; ring<R; ++ring)for (int j=0; j<S; ++j) {
            unsigned a=base+unsigned(ring*(S+1)+j), b=a+S+1;
            triangle(m, a, a+1, b);
            triangle(m, a+1, b+1, b);
        }
    }
    cylinder(m, {0, 0, 0}, {0, 1.25f, 0}, .15f, .075f, 9);
    recalculateNormals(m);
    return m;
}
MeshData makeCoralVariant(int v) {
    if (v<3)return makeGrowthCoral(417u+unsigned(v)*991u, false);
    if (v<6)return makeGrowthCoral(721u+unsigned(v)*531u, true);
    if (v<8)return makeRuffledCoral(811u+unsigned(v)*71u);
    return makeTubeCoral(192u+unsigned(v)*107u);
}

// A stylized extinct-shark silhouette, not a scientifically reconstructed animal.
MeshData makeSharkBody() {
    MeshData mesh = makeSphere(28, 16);
    for (auto& vertex : mesh.vertices) {
        float x = vertex.position.x;
        float taper = x < 0 ? .25f + .75f * (x + 1.0f) : 1.0f - .28f * x;
        vertex.position = Vec3(x * 3.6f, vertex.position.y * .78f * taper,
        vertex.position.z * .72f * taper);
    }
    recalculateNormals(mesh);
    return mesh;
}
MeshData makeSharkTail() {
    MeshData mesh;
    mesh.twoSided = true;
    mesh.vertices = {
    {{0, 0, 0}, {0, 0, 1}, {.5f, .5f}},
    {{-1.5f, 2.0f, 0}, {0, 0, 1}, {0, 1}},
    {{-.80f, .15f, 0}, {0, 0, 1}, {.2f, .5f}},
    {{-1.25f, -1.25f, 0}, {0, 0, 1}, {0, 0}}
    };
    mesh.indices = {0, 1, 2, 0, 2, 3};
    return mesh;
}

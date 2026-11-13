#include "Scene.hpp"
#include "Geometry.hpp"

Chunk generateChunk(ChunkKey key,std::uint32_t seed,int quality) {
    Chunk result;result.key=key;result.terrain=makeTerrainTile(key,seed);
    Random rng(chunkSeed(key,seed));std::vector<Vec2> points;
    const int target[3]={8,14,20};quality=std::max(0,std::min(quality,2));
    for(int attempt=0;attempt<600&&int(points.size())<target[quality];++attempt) {
        Vec2 p{rng.range(-10.25f,10.25f),rng.range(-10.25f,10.25f)};
        double wx=double(key.first)*ChunkSize+p.x,wz=double(key.second)*ChunkSize+p.y;
        if(std::abs(wx+1.8)<2.8&&std::abs(wz+2.6)<2.8)continue;
        bool free=true;for(auto q:points) {float dx=p.x-q.x,dz=p.y-q.y;if(dx*dx+dz*dz<8.4f){free=false;break;}}
        if(free)points.push_back(p);
    }
    const Vec4 palette[]={{.95f,.29f,.34f,1},{.76f,.29f,.71f,1},{.57f,.43f,.95f,1},
        {.26f,.82f,.69f,1},{1,.71f,.26f,1},{.97f,.46f,.28f,1}};
    for(auto p:points) {
        float ground=terrainHeight(double(key.first)*ChunkSize+p.x,double(key.second)*ChunkSize+p.y,seed);
        float size=rng.range(.65f,1.25f);Object coral;
        coral.kind=ObjectKind::Coral;coral.variant=rng.integer(0,CoralVariants-1);
        coral.transform.position={p.x,ground+.16f,p.y};coral.transform.rotation={0,rng.range(0,2*Pi),0};
        coral.transform.size={size*rng.range(.85f,1.15f),size*rng.range(.80f,1.25f),size};
        coral.tint=palette[rng.integer(0,5)];
        coral.boundCenter=coral.transform.position+Vec3(0,1.1f*coral.transform.size.y,0);
        coral.radius=2.5f*std::max({coral.transform.size.x,coral.transform.size.y,coral.transform.size.z});
        result.objects.push_back(coral);++result.coralCount;
        Object rock;rock.kind=ObjectKind::Rock;
        rock.transform.position={p.x,ground,p.y};rock.transform.rotation=coral.transform.rotation;
        rock.transform.size={size*.95f,size*.22f,size*.85f};rock.tint={.85f,.93f,.85f,1};
        rock.boundCenter=rock.transform.position;rock.radius=size*1.4f;result.objects.push_back(rock);
    }
    for(int i=0;i<6;++i) {
        float x=rng.range(-11,11),z=rng.range(-11,11);
        double wx=double(key.first)*ChunkSize+x,wz=double(key.second)*ChunkSize+z;
        if(std::abs(wx+1.8)<3&&std::abs(wz+2.6)<3)continue;
        Object obj;obj.kind=i<4?ObjectKind::Kelp:ObjectKind::Rock;
        obj.transform.position={x,terrainHeight(wx,wz,seed),z};
        obj.transform.rotation={0,rng.range(0,2*Pi),0};
        float f=rng.range(.6f,1.1f);obj.transform.size=Vec3(f);
        obj.tint=i<4?Vec4{.40f,.78f,.57f,1}:Vec4{.64f,.76f,.68f,1};
        obj.boundCenter=obj.transform.position+Vec3(0,i<4?1.7f*f:0,0);
        obj.radius=i<4?2.4f*f:1.4f*f;result.objects.push_back(obj);
    }
    return result;
}

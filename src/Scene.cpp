#include "Scene.hpp"
#include "Geometry.hpp"
#include <algorithm>
#include <cmath>

const StructureInfo& structureInfo(int variant) {
    static const StructureInfo records[StructureVariantCount]={
        {"Broken stone gateway", "Submerged masonry", "Unknown; this is a fictional archaeological-style ruin.",
        "Procedural location; it is not a mapped real-world site.", "Generated for this dive; no historical discovery date.",
        "Varies with the generated seabed.", "The model is a generic ruin. No real civilization or artifact is claimed.", "", ""},
        {"S.S. Thistlegorm", "British cargo steamer", "Sank on October 6, 1941, after a German air attack in the Strait of Gubal, Red Sea.",
        "Strait of Gubal, Red Sea, Egypt.", "Its position appeared on an Admiralty chart in 1948; Jacques Cousteau's 1955 visit brought it to wide attention.",
        "About 12–32 m across the wreck.", "The ship carried military supplies, including trucks, motorcycles, and railway locomotives. The wreck's exact position was already marked on a 1948 chart.",
        "https://www.mdpi.com/2077-1312/12/2/280", "https://thethistlegormproject.com/history/"},
        {"R.M.S. Titanic", "Passenger liner", "Sank on April 15, 1912, after striking an iceberg in the North Atlantic.",
        "About 350 nautical miles off Newfoundland, Canada.", "Located on September 1, 1985, by a joint U.S.–French expedition led by Robert Ballard and Jean-Louis Michel.",
        "About 3,800 m.", "The wreck lies in two main sections on the deep North Atlantic seafloor.",
        "https://www.noaa.gov/office-of-general-counsel/gc-international-section/rms-titanic-history-and-significance", "https://www.whoi.edu/press-room/news-release/titanic-40th/"},
        {"U.S.S. Yorktown (CV-5)", "Aircraft carrier", "Sank on June 7, 1942, after damage sustained during the Battle of Midway and a later submarine attack.",
        "Pacific Ocean, roughly 1,000 miles northwest of Honolulu.", "Discovered on May 19, 1998, by an expedition led by Robert Ballard.",
        "About 5,075 m (16,650 ft).", "The carrier went down after its crew had abandoned ship and a salvage attempt failed.",
        "https://www.history.navy.mil/content/history/museums/nmusn/education/educational-resources/to-the-ends-of-the-earth/undersea-exploration/biography--dr--robert-ballard.html",
        "https://www.history.navy.mil/content/history/nhhc/our-collections/artifacts/rest-and-relaxation/sailor-art/decorated-egg--bill-surgi--uss-yorktown.html"},
        {"Douglas SBD Dauntless", "Carrier-based dive bomber", "One aircraft was aboard Yorktown when the carrier was lost after the Battle of Midway in June 1942.",
        "Inside the wreck of U.S.S. Yorktown in the central Pacific.", "Aircraft remains were documented during NOAA remotely operated vehicle dives in April 2025.",
        "About 5,000 m at the Yorktown site.", "NOAA reported a damaged SBD Dauntless with visible markings. Its possible link to an Enterprise squadron is still an interpretation, not a confirmed identity.",
        "https://oceanexplorer.noaa.gov/expedition-feature/ex2503-yorktown-mission/", "https://oceanexplorer.noaa.gov/photogrammetry/uss-yorktown-cv-5-2025/"},
        {"U.S.S. Lexington (CV-2)", "Aircraft carrier", "Sank on May 8, 1942, after the Battle of the Coral Sea; the damaged carrier was scuttled by U.S.S. Phelps.",
        "Coral Sea, more than 500 miles off Australia's east coast.", "Found on March 4, 2018, by the research vessel Petrel expedition.",
        "About 3,000 m.", "The wreck is broken into several sections along an underwater ridgeline; aircraft were also found nearby.",
        "https://www.history.navy.mil/content/history/nhhc/browse-by-topic/ships/aircraft-carriers/uss-lexington.html", "https://www.navsea.navy.mil/Media/News/Article/2490603/carderocks-unique-capabilities-help-assess-damage-to-sunken-lexington/"},
        {"Japanese submarine I-52", "Long-range submarine", "Lost in June 1944 in the Atlantic after an encounter with U.S. aircraft west of the Cape Verde Islands.",
        "North Atlantic, near the wartime aircraft-attack area west of Cape Verde.", "Located by American researchers in 1995; the search team used archival logs to refine the position.",
        "About 5,200 m (17,000 ft).", "I-52 was on a long voyage between Japan and occupied Europe. The U.S. Naval History and Heritage Command documents its loss and 1995 discovery.",
        "https://www.history.navy.mil/about-us/leadership/director/directors-corner/h-grams/h-gram-033/h-033-1.html", "https://nauticos.com/ocean-discovery/i-52/"},
        {"Coastal fishing boat", "Small working vessel", "Unknown; this generated specimen has no assigned real sinking event.",
        "Procedural location in this fictional survey world.", "First sighted by the player in the generated scene; this is not a real archaeological discovery.",
        "Varies with the generated seabed.", "The shape is a composite of small working boats. Historical coordinates and a discovery date are intentionally not invented.",
        "", ""}
    };
    const int index=std::max(0, std::min(variant, StructureVariantCount-1));
    return records[index];
}

static std::uint32_t structurePickId(ChunkKey cell, std::uint32_t seed) {
    const std::uint64_t value=hash64(chunkSeed(cell, seed+991u));
    return StructurePickBase|static_cast<std::uint32_t>(value&0x3fffffffu);
}

static void finishBounds(Chunk& chunk) {
    Vec3 lo(1e9f),hi(-1e9f);
    for(const auto& v:chunk.terrain.vertices) { lo=glm::min(lo,v.position);hi=glm::max(hi,v.position); }
    chunk.terrainCenter=(lo+hi)*.5f;
    chunk.terrainRadius=length(hi-lo)*.5f+.1f;
    for(const auto& o:chunk.objects) { lo=glm::min(lo,o.boundCenter-Vec3(o.radius));hi=glm::max(hi,o.boundCenter+Vec3(o.radius)); }
    chunk.boundsCenter=(lo+hi)*.5f;
    chunk.boundsRadius=length(hi-lo)*.5f+.1f;
}
static void addVisualGardens(Chunk& chunk, std::uint32_t seed, int quality) {
    Random rng(chunkSeed(chunk.key,seed+1301u));
    const double ox=double(chunk.key.first)*ChunkSize,oz=double(chunk.key.second)*ChunkSize;
    const int attempts[]={45,80,115};
    std::vector<Vec2> occupied;
    const Vec4 palettes[3][4]={
        {{.95f,.34f,.20f,1},{1.0f,.64f,.22f,1},{.82f,.24f,.35f,1},{.95f,.49f,.36f,1}},
        {{.25f,.67f,.71f,1},{.45f,.37f,.83f,1},{.76f,.34f,.68f,1},{.36f,.81f,.62f,1}},
        {{.79f,.45f,.24f,1},{.94f,.69f,.36f,1},{.49f,.65f,.47f,1},{.73f,.31f,.43f,1}}
    };
    for(int i=0;i<attempts[quality];++i) {
        const float x=rng.range(-10.9f,10.9f),z=rng.range(-10.9f,10.9f);
        const double wx=ox+x,wz=oz+z;
        if(std::abs(wx+1.8)<2.5&&std::abs(wz+2.6)<2.5)continue;
        const float weight=reefGardenWeight(wx,wz,seed);
        if(rng.unit()>weight)continue;
        bool clear=true;
        for(const auto& q:occupied)if((q.x-x)*(q.x-x)+(q.y-z)*(q.y-z)<2.0f) { clear=false;break; }
        if(!clear)continue;
        occupied.push_back({x,z});
        const int biome=reefPalette(wx,wz,seed);
        const float floor=terrainHeight(wx,wz,seed,true);
        Object coral;
        coral.kind=ObjectKind::Coral;
        // Large plates and tube clumps are inexpensive; sparse tall fans frame the view.
        const float pick=rng.unit();
        coral.variant=pick<.24f?rng.integer(0,2):(pick<.45f?rng.integer(3,5):(pick<.76f?rng.integer(6,7):rng.integer(8,9)));
        const float size=rng.range(.58f,1.38f);
        const float tall=coral.variant<6 ? rng.range(.90f,1.70f) : rng.range(.65f,1.05f);
        coral.transform.position={x,floor-.035f,z};
        coral.transform.rotation={rng.range(-.08f,.08f),rng.range(0,2*Pi),rng.range(-.06f,.06f)};
        coral.transform.size={size*rng.range(.85f,1.25f),size*tall,size};
        coral.tint=palettes[biome][rng.integer(0,3)];
        coral.boundCenter=coral.transform.position+Vec3(0,1.4f*coral.transform.size.y,0);
        coral.radius=3.2f*std::max({coral.transform.size.x,coral.transform.size.y,coral.transform.size.z});
        chunk.objects.push_back(coral);++chunk.coralCount;
        Object rock;rock.kind=ObjectKind::Rock;
        rock.transform.position={x,floor-.10f,z};
        rock.transform.rotation={0,coral.transform.rotation.y,0};
        rock.transform.size={size*.75f,size*.22f,size*.66f};
        rock.tint={.67f,.73f,.62f,1};rock.boundCenter=rock.transform.position;rock.radius=size*1.5f;
        if(i%3==0)chunk.objects.push_back(rock);
    }
    // Cohesive vegetation beds with a clear sandy swimming lane.
    const int plants[]={14,26,38};
    for(int i=0;i<plants[quality];++i) {
        const float x=rng.range(-11.0f,11.0f),z=rng.range(-11.0f,11.0f);
        const double wx=ox+x,wz=oz+z;
        if(reefGardenWeight(wx,wz,seed)<.20f)continue;
        const bool tall=wx>9&&wx<30&&wz<8&&wz>-40 ? rng.unit()<.68f : rng.unit()<.16f;
        const float size=rng.range(.62f,1.38f);
        Object plant;plant.kind=tall?ObjectKind::Kelp:ObjectKind::Seagrass;
        plant.transform.position={x,terrainHeight(wx,wz,seed,true)-.03f,z};
        plant.transform.rotation={0,rng.range(0,2*Pi),0};
        plant.transform.size={size,size*rng.range(.85f,1.35f),size};
        plant.tint=tall?Vec4{.44f,.66f,.28f,1}:Vec4{.30f,.60f,.45f,1};
        plant.boundCenter=plant.transform.position+Vec3(0,tall?2.4f*plant.transform.size.y:.45f*size,0);
        plant.radius=tall?4.0f*std::max(size,plant.transform.size.y):1.8f*size;
        chunk.objects.push_back(plant);
    }
    // Low-cost stacked boulders break up silhouettes along the valley's banks.
    for(int i=0;i<5;++i) {
        const float x=rng.range(-10,10),z=rng.range(-10,10);
        const double wx=ox+x,wz=oz+z;
        if(reefGardenWeight(wx,wz,seed)<.26f)continue;
        const float scaleValue=rng.range(1.1f,2.6f);
        Object rock;rock.kind=ObjectKind::Rock;
        rock.transform.position={x,terrainHeight(wx,wz,seed,true)+scaleValue*.35f,z};
        rock.transform.rotation={rng.range(-.18f,.18f),rng.range(0,2*Pi),rng.range(-.18f,.18f)};
        rock.transform.size={scaleValue,scaleValue*rng.range(.7f,1.5f),scaleValue*.85f};
        rock.tint={.64f,.69f,.58f,1};rock.boundCenter=rock.transform.position;
        rock.radius=1.55f*std::max({rock.transform.size.x,rock.transform.size.y,rock.transform.size.z});
        chunk.objects.push_back(rock);
    }
    if(chunk.key==ChunkKey{0,-1}) {
        Object arch;arch.kind=ObjectKind::Arch;
        arch.transform.position={-2.0f,terrainHeight(-2.0,-25.0,seed,true)+.65f,-1.0f};
        arch.transform.rotation={0,.13f,0};arch.transform.size=Vec3(1.0f);
        arch.tint={.61f,.66f,.57f,1};arch.boundCenter=arch.transform.position+Vec3(0,2.0f,0);arch.radius=9;
        chunk.objects.push_back(arch);
    }
}
Chunk generateChunk(ChunkKey key, std::uint32_t seed, int quality, bool enhanced) {
    Chunk result;
    result.key=key;
    result.terrain=makeTerrainTile(key, seed, enhanced);
    Random rng(chunkSeed(key, seed));
    std::vector<Vec2> points;
    const int target[3]={2, 4, 6};
    quality=std::max(0, std::min(quality, 2));
    if (!enhanced) {
    for (int attempt=0; attempt<600&&int(points.size())<target[quality]; ++attempt) {
        Vec2 p{rng.range(-10.25f, 10.25f), rng.range(-10.25f, 10.25f)};
        const double wx=double(key.first)*ChunkSize+p.x, wz=double(key.second)*ChunkSize+p.y;
        if (std::abs(wx+1.8)<2.8&&std::abs(wz+2.6)<2.8)continue;
        if (rng.unit()>reefIntensity(wx, wz, seed))continue;
        bool free=true;
        for (auto q:points) {
            const float dx=p.x-q.x, dz=p.y-q.y;
            if (dx*dx+dz*dz<8.4f) {
                free=false;
                break;
            }
        }
        if (free)points.push_back(p);
    }
    const Vec4 palette[]={{.95f, .29f, .34f, 1}, {.76f, .29f, .71f, 1}, {.57f, .43f, .95f, 1},
    {.26f, .82f, .69f, 1}, {1, .71f, .26f, 1}, {.97f, .46f, .28f, 1}};
    for (auto p:points) {
        const float ground=terrainHeight(double(key.first)*ChunkSize+p.x, double(key.second)*ChunkSize+p.y, seed, enhanced);
        const float size=rng.range(.65f, 1.25f);
        Object coral;
        coral.kind=ObjectKind::Coral;
        coral.variant=rng.integer(0, CoralVariants-1);
        coral.transform.position={p.x, ground+.16f, p.y};
        coral.transform.rotation={0, rng.range(0, 2*Pi), 0};
        coral.transform.size={size*rng.range(.85f, 1.15f), size*rng.range(.80f, 1.25f), size};
        coral.tint=palette[rng.integer(0, 5)];
        coral.boundCenter=coral.transform.position+Vec3(0, 1.1f*coral.transform.size.y, 0);
        coral.radius=2.5f*std::max({coral.transform.size.x, coral.transform.size.y, coral.transform.size.z});
        result.objects.push_back(coral);
        ++result.coralCount;
        Object rock;
        rock.kind=ObjectKind::Rock;
        rock.transform.position={p.x, ground, p.y};
        rock.transform.rotation=coral.transform.rotation;
        rock.transform.size={size*.95f, size*.22f, size*.85f};
        rock.tint={.66f, .73f, .72f, 1};
        rock.boundCenter=rock.transform.position;
        rock.radius=size*1.4f;
        result.objects.push_back(rock);
    }

    // Rocky patches remain visible between the much less common coral gardens.
    for (int i=0; i<4; ++i)if (rng.unit()<.20f) {
        const float x=rng.range(-11, 11), z=rng.range(-11, 11);
        const double wx=double(key.first)*ChunkSize+x, wz=double(key.second)*ChunkSize+z;
        const float size=rng.range(.65f, 1.6f);
        Object rock;
        rock.kind=ObjectKind::Rock;
        rock.transform.position={x, terrainHeight(wx, wz, seed, enhanced)+size*.25f, z};
        rock.transform.rotation={rng.range(-.12f, .12f), rng.range(0, 2*Pi), rng.range(-.12f, .12f)};
        rock.transform.size={size*rng.range(.8f, 1.4f), size*rng.range(.7f, 1.1f), size*rng.range(.8f, 1.5f)};
        rock.tint={.56f, .66f, .68f, 1};
        rock.boundCenter=rock.transform.position;
        rock.radius=1.5f*size;
        result.objects.push_back(rock);
    }

    // Tall kelp and short seagrass form broad beds instead of covering every tile.
    for (int i=0; i<14; ++i) {
        const float x=rng.range(-11, 11), z=rng.range(-11, 11);
        const double wx=double(key.first)*ChunkSize+x, wz=double(key.second)*ChunkSize+z;
        const float bed=algaeIntensity(wx, wz, seed);
        if (rng.unit()>bed*.58f)continue;
        const bool tallKelp=rng.unit()<.24f;
        const float size=rng.range(.55f, 1.2f);
        Object plant;
        plant.kind=tallKelp?ObjectKind::Kelp:ObjectKind::Seagrass;
        plant.transform.position={x, terrainHeight(wx, wz, seed, enhanced), z};
        plant.transform.rotation={0, rng.range(0, 2*Pi), 0};
        plant.transform.size=Vec3(size);
        plant.tint=tallKelp?Vec4{.38f, .57f, .31f, 1}:Vec4{.35f, .66f, .39f, 1};
        plant.boundCenter=plant.transform.position+Vec3(0, tallKelp?1.7f*size:.38f*size, 0);
        plant.radius=tallKelp?2.5f*size:1.45f*size;
        result.objects.push_back(plant);
    }

    } else {
        addVisualGardens(result, seed, quality);
    }

    // Human debris collects in separated patches; the individual pieces are small.
    if (rng.unitDouble()<.105) {
        const int pieces=rng.integer(3, 7);
        const Vec4 debrisTints[]={{.54f, .48f, .35f, 1}, {.37f, .48f, .51f, 1}, {.26f, .29f, .29f, 1},
        {.39f, .43f, .36f, 1}, {.62f, .34f, .22f, 1}};
        for (int i=0; i<pieces; ++i) {
            const float x=rng.range(-4.5f, 4.5f), z=rng.range(-4.5f, 4.5f);
            const double wx=double(key.first)*ChunkSize+x, wz=double(key.second)*ChunkSize+z;
            const float size=rng.range(.45f, 1.05f);
            Object debris;
            debris.kind=ObjectKind::Debris;
            debris.variant=rng.integer(0, 4);
            debris.transform.position={x, terrainHeight(wx, wz, seed, enhanced)+.06f, z};
            debris.transform.rotation={rng.range(-.2f, .2f), rng.range(0, 2*Pi), rng.range(-.2f, .2f)};
            debris.transform.size=Vec3(size);
            debris.tint=debrisTints[debris.variant];
            debris.boundCenter=debris.transform.position+Vec3(0, .18f*size, 0);
            debris.radius=1.8f*size;
            result.objects.push_back(debris);
        }
    }

    // A few steep chunks shed a compact boulder field down the local slope.
    if (rng.unitDouble()<.0125) {
        float bestSlope=0, bestDx=0, bestDz=0;
        Vec2 slideCenter{};
        for (int attempt=0; attempt<28; ++attempt) {
            Vec2 p{rng.range(-10.0f, 10.0f), rng.range(-10.0f, 10.0f)};
            const double wx=double(key.first)*ChunkSize+p.x, wz=double(key.second)*ChunkSize+p.y;
            constexpr double sample=.8;
            const float dx=float((terrainHeight(wx+sample, wz, seed, enhanced)-terrainHeight(wx-sample, wz, seed, enhanced))/(2*sample));
            const float dz=float((terrainHeight(wx, wz+sample, seed, enhanced)-terrainHeight(wx, wz-sample, seed, enhanced))/(2*sample));
            const float slope=std::sqrt(dx*dx+dz*dz);
            if (slope>bestSlope) {
                bestSlope=slope;
                bestDx=dx;
                bestDz=dz;
                slideCenter=p;
            }
        }
        if (bestSlope>.11f) {
            const float downhillX=-bestDx/bestSlope, downhillZ=-bestDz/bestSlope;
            const int boulderCount=rng.integer(5, 9);
            const Vec4 stoneTints[]={{.63f, .70f, .68f, 1}, {.75f, .69f, .57f, 1}, {.57f, .64f, .64f, 1}};
            for (int i=0; i<boulderCount; ++i) {
                const float along=float(i)-float(boulderCount-1)*.5f;
                const float x=slideCenter.x+downhillX*along*1.1f+rng.range(-.8f, .8f);
                const float z=slideCenter.y+downhillZ*along*1.1f+rng.range(-.8f, .8f);
                const double wx=double(key.first)*ChunkSize+x, wz=double(key.second)*ChunkSize+z;
                const float size=rng.range(.65f, 1.35f);
                Object boulder;
                boulder.kind=ObjectKind::Rockslide;
                boulder.transform.position={x, terrainHeight(wx, wz, seed, enhanced)+size*.32f, z};
                boulder.transform.rotation={rng.range(-.08f, .08f), rng.range(0, 2*Pi), rng.range(-.08f, .08f)};
                boulder.transform.size={size*rng.range(.85f, 1.35f), size*rng.range(.62f, .94f), size*rng.range(.8f, 1.3f)};
                boulder.tint=stoneTints[rng.integer(0, 2)];
                boulder.boundCenter=boulder.transform.position;
                boulder.radius=1.55f*std::max({boulder.transform.size.x, boulder.transform.size.y, boulder.transform.size.z});
                result.objects.push_back(boulder);
            }
        }
    }

    // One possible site per large region keeps wrecks hundreds of metres apart.
    constexpr double structureRegion=520.0;
    const double centerX=double(key.first)*ChunkSize, centerZ=double(key.second)*ChunkSize;
    const auto firstX=static_cast<std::int64_t>(std::floor((centerX-ChunkSize*.5)/structureRegion));
    const auto lastX=static_cast<std::int64_t>(std::floor((centerX+ChunkSize*.5)/structureRegion));
    const auto firstZ=static_cast<std::int64_t>(std::floor((centerZ-ChunkSize*.5)/structureRegion));
    const auto lastZ=static_cast<std::int64_t>(std::floor((centerZ+ChunkSize*.5)/structureRegion));
    for (std::int64_t gz=firstZ; gz<=lastZ; ++gz)for (std::int64_t gx=firstX; gx<=lastX; ++gx) {
        const bool startingRegion=gx==0&&gz==0;
        if (!startingRegion&&noiseValue(gx, gz, seed+809u)>.32)continue;
        const double wx=startingRegion?-5.0:(double(gx)+.5)*structureRegion+(noiseValue(gx, gz, seed+811u)-.5)*140.0;
        const double wz=startingRegion?-9.0:(double(gz)+.5)*structureRegion+(noiseValue(gx, gz, seed+821u)-.5)*140.0;
        const auto ownerX=static_cast<std::int64_t>(std::floor((wx+ChunkSize*.5)/ChunkSize));
        const auto ownerZ=static_cast<std::int64_t>(std::floor((wz+ChunkSize*.5)/ChunkSize));
        if (ownerX!=key.first||ownerZ!=key.second)continue;
        const std::uint32_t siteSeed=chunkSeed({gx, gz}, seed+827u);
        const int variant=startingRegion?int(StructureVariant::OceanLiner):static_cast<int>(siteSeed%std::uint32_t(StructureVariantCount));
        Object wreck;
        wreck.kind=ObjectKind::Structure;
        wreck.variant=variant;
        wreck.pickId=structurePickId({gx, gz}, seed);
        const float localX=float(wx-centerX), localZ=float(wz-centerZ);
        wreck.transform.position={localX, terrainHeight(wx, wz, seed, enhanced), localZ};
        wreck.transform.rotation={0, float(noiseValue(gx, gz, seed+829u)*2.0*Pi), 0};
        const float size=.86f+float(noiseValue(gx, gz, seed+839u))*.38f;
        wreck.transform.size={size, size, size};
        wreck.tint=variant==int(StructureVariant::Ruin)?Vec4{.68f, .75f, .72f, 1}:Vec4{.48f, .40f, .31f, 1};
        wreck.boundCenter=wreck.transform.position+Vec3(0, 2.4f*size, 0);
        wreck.radius=(variant==int(StructureVariant::AircraftCarrier)||variant==int(StructureVariant::LexingtonCarrier)||variant==int(StructureVariant::OceanLiner)?15.0f:variant==int(StructureVariant::CargoShip)?12.0f:variant==int(StructureVariant::Aircraft)?8.0f:variant==int(StructureVariant::Submarine)?10.0f:6.0f)*size;
        result.objects.push_back(wreck);
    }
    finishBounds(result);
    return result;
}

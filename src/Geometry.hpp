#pragma once
#include "MeshData.hpp"
#include "World.hpp"

MeshData makeSphere(int slices=24, int rings=16);
MeshData makeTubeCoral(std::uint32_t seed=93);
MeshData makeRock();
MeshData makeUnderwaterStructure();
MeshData makeStructure(int variant);
MeshData makeDebris(int variant);
MeshData makeKelp();
MeshData makeSeagrass();
MeshData makeTail();
MeshData makeFin();
MeshData makeRing();
MeshData makeTerrainTile(ChunkKey key, std::uint32_t seed);
MeshData makeGrowthCoral(std::uint32_t seed, bool fan=false);
MeshData makeRuffledCoral(std::uint32_t seed);
MeshData makeCoralVariant(int variant);
void recalculateNormals(MeshData& mesh);
MeshData makeSharkBody();
MeshData makeSharkTail();

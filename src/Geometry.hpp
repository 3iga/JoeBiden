#pragma once
#include "MeshData.hpp"
#include "World.hpp"

MeshData makeSphere(int slices=24,int rings=16);
MeshData makeSeabed();
MeshData makeBranchCoral();
MeshData makeTubeCoral(std::uint32_t seed=93);
MeshData makePlateCoral();
MeshData makeRock();
MeshData makeKelp();
MeshData makeTail();
MeshData makeFin();
MeshData makeRing();
MeshData makeTerrainTile(ChunkKey key,std::uint32_t seed);
MeshData makeGrowthCoral(std::uint32_t seed,bool fan=false);
MeshData makeRuffledCoral(std::uint32_t seed);
MeshData makeCoralVariant(int variant);
void recalculateNormals(MeshData& mesh);

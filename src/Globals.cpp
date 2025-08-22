#include "Globals.h"

// 初始化静态成员
std::unordered_map<std::pair<int,int>, std::shared_ptr<Chunk>, pair_hash> gChunks;
FastNoiseLite noise;
int floorDiv(int a, int b) {
    return (a >= 0) ? (a / b) : ((a - b + 1) / b);
}

int mod(int a, int b) {
    int m = a % b;
    return (m >= 0) ? m : (m + b); // 保证结果在 [0, b-1]
}

Block* getBlockAtGlobal(int x, int y, int z) {
    int chunkX = floorDiv(x, CHUNK_SIZE);
    int chunkZ = floorDiv(z, CHUNK_SIZE);

    auto it = gChunks.find({chunkX, chunkZ});
    if(it == gChunks.end()) return nullptr;

    int localX = mod(x, CHUNK_SIZE);
    int localZ = mod(z, CHUNK_SIZE);
    return it->second->getBlockAtLocal(localX, y, localZ);
}

int Globals::playerChunkX = 0;
int Globals::playerChunkZ = 0;
float Globals::deltaTime = 0.0f;
float Globals::lastFrame = 0.0f;

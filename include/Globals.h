#pragma once
#include <unordered_map>
#include <memory>
#include <utility>
#include "Chunk.h"
#include <FastNoiseLite.h>

struct pair_hash
{
    template <class T1, class T2>
    std::size_t operator()(const std::pair<T1, T2> &p) const
    {
        return std::hash<T1>()(p.first) ^ std::hash<T2>()(p.second);
    }
};

// 全局 Chunk 管理器
extern std::unordered_map<std::pair<int, int>, std::shared_ptr<Chunk>, pair_hash> gChunks;
extern FastNoiseLite noise;

// 全局查询函数
Block *getBlockAtGlobal(int x, int y, int z);
// 全局管理类
class Globals
{
public:
    // ----------------- Chunk 相关 -----------------
    // key: (chunkX, chunkZ)
    // 用 pair<int,int> 做 key，需要哈希函数

    // ----------------- 玩家位置 -----------------
    static int playerChunkX;
    static int playerChunkZ;

    // ----------------- 其他全局变量示例 -----------------
    static float deltaTime;
    static float lastFrame;

private:
    Globals() = default; // 禁止实例化
};

#pragma once
#include <unordered_map>
#include "Block.h"
#include <gl.h>
#include <string>

struct TextureSet {
    unsigned int top;
    unsigned int bottom;
    unsigned int side;
};

class TextureManager {
public:
    static void init(); // 初始化所有方块纹理
    static const TextureSet& get(BlockType type);

private:
    static std::unordered_map<BlockType, TextureSet> textures;

    static unsigned int loadTexture(const std::string& path);
};

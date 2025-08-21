#ifndef BLOCKMANAGER_H
#define BLOCKMANAGER_H

#include <vector>
#include "Block.h"
#include "Shader.h"
#include <glm.hpp>


class BlockManager {
public:
    void addBlock(BlockType type, glm::vec3 position);
    void renderAll(Shader &shader);
private:
    // std::vector<BlockInstance> blocks;
};

#endif

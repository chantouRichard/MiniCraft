#ifndef CHUNK_H
#define CHUNK_H

#include <vector>
#include <glm.hpp>
#include "Block.h"
#include "Shader.h"
#include <memory>
#include <vector>

struct BlockInstance {
    std::shared_ptr<Block> block;
    glm::vec3 position;
};
#define CHUNK_SIZE 16

class Chunk {
public:
    Chunk(glm::vec3 origin);
    // ~Chunk() { for(auto& b: blocks) b.first.clear(); }

    void generate();
    void render(Shader& shader);

private:
    glm::vec3 origin;
    std::vector<BlockInstance> blocks;


    void updateFaceVisibility();
};

#endif

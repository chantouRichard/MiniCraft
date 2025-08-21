#include "Chunk.h"
#include <gtc/matrix_transform.hpp>

Chunk::Chunk(glm::vec3 origin) : origin(origin) {}

void Chunk::generate() {
    blocks.clear();

    // 生成底层石头
    for (int x = 0; x < 8*CHUNK_SIZE; x++) {
        for (int z = 0; z < 8*CHUNK_SIZE; z++) {
            glm::vec3 pos = origin + glm::vec3(x, -1, z);
            auto grassBlock = std::make_shared<Block>(BlockType::Stone);
            blocks.push_back({ grassBlock, pos });

        }
    }

    // 生成草方块层
    for (int x = 0; x < 8*CHUNK_SIZE; x++) {
        for (int z = 0; z < 8*CHUNK_SIZE; z++) {
            glm::vec3 pos = origin + glm::vec3(x, 0, z);
            auto grassBlock = std::make_shared<Block>(BlockType::Grass);
            blocks.push_back({ grassBlock, pos });

        }
    }

    updateFaceVisibility();
}

// 简单面剔除（仅检查同一 Chunk 内方块）
void Chunk::updateFaceVisibility() {
    auto getBlockAt = [&](int x, int y, int z) -> Block* {
        for (auto& b : blocks) {
            glm::vec3 p = b.position;
            if ((int)p.x == x + (int)origin.x && (int)p.y == y + (int)origin.y && (int)p.z == z + (int)origin.z)
                return b.block.get();
        }
        return nullptr;
    };

    for (auto& b : blocks) {
        glm::vec3 p = b.position;
        Block* block = b.block.get();

        // 上
        block->visibleFaces[Face::TOP] = getBlockAt((int)p.x, (int)p.y + 1, (int)p.z) == nullptr;
        // 下
        block->visibleFaces[Face::BOTTOM] = getBlockAt((int)p.x, (int)p.y - 1, (int)p.z) == nullptr;
        // 左
        block->visibleFaces[Face::LEFT] = getBlockAt((int)p.x - 1, (int)p.y, (int)p.z) == nullptr;
        // 右
        block->visibleFaces[Face::RIGHT] = getBlockAt((int)p.x + 1, (int)p.y, (int)p.z) == nullptr;
        // 前
        block->visibleFaces[Face::FRONT] = getBlockAt((int)p.x, (int)p.y, (int)p.z + 1) == nullptr;
        // 后
        block->visibleFaces[Face::BACK] = getBlockAt((int)p.x, (int)p.y, (int)p.z - 1) == nullptr;
    }
}

void Chunk::render(Shader& shader) {
    for (auto& instance : blocks) {
        glm::mat4 model = glm::translate(glm::mat4(1.0f), instance.position);
        shader.setMat4("model", model);
        instance.block->render(shader);
    }
}


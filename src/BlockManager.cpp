#include "BlockManager.h"
#include <gl.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

void BlockManager::addBlock(BlockType type, glm::vec3 position) {
    // blocks.push_back({ Block(type), position });
}

void BlockManager::renderAll(Shader &shader) {
    // for(auto &instance : blocks) {
    //     // 这里可以在渲染前把model矩阵平移到 position
    //     glm::mat4 model = glm::translate(glm::mat4(1.0f), instance.position);
    //     shader.setMat4("model", model);
    //     instance.block.render(shader);
    // }
}

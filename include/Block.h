#ifndef BLOCK_H
#define BLOCK_H

#include <gl.h>
#include <glm.hpp>
#include "Shader.h"
#include "BlockType.h"
enum Face {
    TOP,
    BOTTOM,
    LEFT,
    RIGHT,
    FRONT,
    BACK
};

class Block {
public:
    Block(BlockType type);
    ~Block() { clear(); }

    void render(Shader& shader);
    void clear();

    // 面剔除标记
    bool visibleFaces[6] = { true, true, true, true, true, true };
    static void initSharedMesh(); // 初始化共享顶点数据
    GLuint getTextureForFace(Face f) const;

private:
    void setupMesh();
    void loadTextures();

    static unsigned int VAO, VBO;
    unsigned int textureTop, textureBottom, textureSide;
    BlockType type;
};

#endif

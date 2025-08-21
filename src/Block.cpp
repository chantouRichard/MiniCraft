#include "Block.h"
#include <stb_image.h>
#include <iostream>
#include "TextureManager.h"

unsigned int Block::VAO = 0;
unsigned int Block::VBO = 0;

Block::Block(BlockType type) : type(type) {
    // loadTextures();
    // 从纹理管理器获取共享纹理
    TextureSet ts = TextureManager::get(type);
    textureTop    = ts.top;
    textureBottom = ts.bottom;
    textureSide   = ts.side;
}

void Block::loadTextures() {
    auto loadTexture = [](const char* path) {
        unsigned int tex;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        int w, h, nrChannels;
        unsigned char* data = stbi_load(path, &w, &h, &nrChannels, 0);
        if (data) {
            GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
            glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        } else {
            std::cerr << "Failed to load texture: " << path << std::endl;
        }
        stbi_image_free(data);
        return tex;
    };

    if (type == BlockType::Grass) {
        textureTop    = loadTexture("assets/textures/grass_top.png");
        textureBottom = loadTexture("assets/textures/grass_bottom.png");
        textureSide   = loadTexture("assets/textures/grass_side.png");
    }
    else if (type == BlockType::Dirt) {
        textureTop = textureBottom = textureSide = loadTexture("assets/textures/dirt.png");
    }
    else if (type == BlockType::Stone) {
        textureTop = textureBottom = textureSide = loadTexture("assets/textures/stone.png");
    }
}

void Block::render(Shader& shader) {
    shader.use();
    glBindVertexArray(VAO);

    if (visibleFaces[Face::TOP]) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureTop);
        shader.setInt("texture1", 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    if (visibleFaces[Face::BOTTOM]) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureBottom);
        shader.setInt("texture1", 0);
        glDrawArrays(GL_TRIANGLES, 6, 6);
    }
    for(int i = 0; i < 4; ++i) { // 四个侧面
        if(visibleFaces[i + 2]) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureSide);
            shader.setInt("texture1", 0);
            glDrawArrays(GL_TRIANGLES, 12 + i * 6, 6);
        }
    }
    glBindVertexArray(0);
}

void Block::clear() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

void Block::initSharedMesh() {
    if (VAO != 0) return; // 已经初始化过了

        float vertices[] = {
        // 顶部
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
         0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
         0.5f, 0.5f,  0.5f, 1.0f, 0.0f,
         0.5f, 0.5f,  0.5f, 1.0f, 0.0f,
        -0.5f, 0.5f,  0.5f, 0.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
        // 底部
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
         0.5f, -0.5f, -0.5f, 1.0f, 1.0f,
         0.5f, -0.5f,  0.5f, 1.0f, 0.0f,
         0.5f, -0.5f,  0.5f, 1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        
        // 左面
        -0.5f, -0.5f, -0.5f, 1.0f, 1.0f,
        -0.5f, 0.5f, -0.5f, 1.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 1.0f, 1.0f,
        // 右面
         0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
         0.5f, 0.5f, -0.5f, 0.0f, 0.0f,
         0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
         0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
         0.5f, -0.5f, 0.5f, 1.0f, 1.0f,
         0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
         // 前面
        -0.5f, -0.5f, 0.5f, 0.0f, 1.0f,
         0.5f, -0.5f, 0.5f, 1.0f, 1.0f,
         0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
         0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 1.0f,
        // 后面
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
         0.5f, -0.5f, -0.5f, 1.0f, 1.0f,
         0.5f, 0.5f, -0.5f, 1.0f, 0.0f,
         0.5f, 0.5f, -0.5f, 1.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 位置
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // 纹理坐标
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}
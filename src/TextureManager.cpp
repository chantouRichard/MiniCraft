#include "TextureManager.h"
#include <iostream>
#include <stb_image.h>

std::unordered_map<BlockType, TextureSet> TextureManager::textures;

void TextureManager::init() {
    // 草方块
    textures[BlockType::Grass] = {
        loadTexture("assets/textures/grass_top.png"),
        loadTexture("assets/textures/grass_bottom.png"),
        loadTexture("assets/textures/grass_side.png")
    };

    // 石头
    textures[BlockType::Stone] = {
        loadTexture("assets/textures/stone.png"),
        loadTexture("assets/textures/stone.png"),
        loadTexture("assets/textures/stone.png")
    };

    // 泥土
    textures[BlockType::Dirt] = {
        loadTexture("assets/textures/dirt.png"),
        loadTexture("assets/textures/dirt.png"),
        loadTexture("assets/textures/dirt.png")
    };
}

const TextureSet& TextureManager::get(BlockType type) {
    return textures.at(type);
}

unsigned int TextureManager::loadTexture(const std::string& path) {
    unsigned int tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int w, h, channels;
    unsigned char* data = stbi_load(path.c_str(), &w, &h, &channels, 0);
    if (data) {
        GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cerr << "Failed to load texture: " << path << std::endl;
    }
    stbi_image_free(data);
    return tex;
}

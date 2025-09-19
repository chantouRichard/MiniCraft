#include "Chunk.h"
#include <gtc/matrix_transform.hpp>
#include <unordered_map>
#include <iostream>
#include "Globals.h"
#include <chrono>
using Clock = std::chrono::high_resolution_clock;
// ===== 面模板（与 Block::initSharedMesh 的顺序/UV一致） =====
const Chunk::Vtx Chunk::FACE_TOP[6] = {
    {0.0f, 1.0f, 0.0f, 0.0f, 1.0f},
    {1.0f, 1.0f, 0.0f, 1.0f, 1.0f},
    {1.0f, 1.0f, 1.0f, 1.0f, 0.0f},
    {1.0f, 1.0f, 1.0f, 1.0f, 0.0f},
    {0.0f, 1.0f, 1.0f, 0.0f, 0.0f},
    {0.0f, 1.0f, 0.0f, 0.0f, 1.0f},
};
const Chunk::Vtx Chunk::FACE_BOTTOM[6] = {
    {0.0f, 0.0f, 0.0f, 0.0f, 1.0f},
    {1.0f, 0.0f, 0.0f, 1.0f, 1.0f},
    {1.0f, 0.0f, 1.0f, 1.0f, 0.0f},
    {1.0f, 0.0f, 1.0f, 1.0f, 0.0f},
    {0.0f, 0.0f, 1.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 0.0f, 0.0f, 1.0f},
};
const Chunk::Vtx Chunk::FACE_LEFT[6] = {
    {0.0f, 0.0f, 0.0f, 1.0f, 1.0f},
    {0.0f, 1.0f, 0.0f, 1.0f, 0.0f},
    {0.0f, 1.0f, 1.0f, 0.0f, 0.0f},
    {0.0f, 1.0f, 1.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 1.0f, 0.0f, 1.0f},
    {0.0f, 0.0f, 0.0f, 1.0f, 1.0f},
};
const Chunk::Vtx Chunk::FACE_RIGHT[6] = {
    {1.0f, 0.0f, 0.0f, 0.0f, 1.0f},
    {1.0f, 1.0f, 0.0f, 0.0f, 0.0f},
    {1.0f, 1.0f, 1.0f, 1.0f, 0.0f},
    {1.0f, 1.0f, 1.0f, 1.0f, 0.0f},
    {1.0f, 0.0f, 1.0f, 1.0f, 1.0f},
    {1.0f, 0.0f, 0.0f, 0.0f, 1.0f},
};
const Chunk::Vtx Chunk::FACE_FRONT[6] = {
    {0.0f, 0.0f, 1.0f, 0.0f, 1.0f},
    {1.0f, 0.0f, 1.0f, 1.0f, 1.0f},
    {1.0f, 1.0f, 1.0f, 1.0f, 0.0f},
    {1.0f, 1.0f, 1.0f, 1.0f, 0.0f},
    {0.0f, 1.0f, 1.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 1.0f, 0.0f, 1.0f},
};
const Chunk::Vtx Chunk::FACE_BACK[6] = {
    {0.0f, 0.0f, 0.0f, 0.0f, 1.0f},
    {1.0f, 0.0f, 0.0f, 1.0f, 1.0f},
    {1.0f, 1.0f, 0.0f, 1.0f, 0.0f},
    {1.0f, 1.0f, 0.0f, 1.0f, 0.0f},
    {0.0f, 1.0f, 0.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 0.0f, 0.0f, 1.0f},
};

void Chunk::appendFace(std::vector<float> &out, const glm::vec3 &base, const Vtx face[6])
{
    out.reserve(out.size() + 6 * 5);
    for (int i = 0; i < 6; ++i)
    {
        out.push_back(face[i].x + base.x);
        out.push_back(face[i].y + base.y);
        out.push_back(face[i].z + base.z);
        out.push_back(face[i].u);
        out.push_back(face[i].v);
    }
}

Chunk::Chunk(glm::vec3 origin) : origin(origin) {}
bool Chunk::neighborsReady() {
    static const std::vector<std::pair<int,int>> dirs = {
        {1,0}, {-1,0}, {0,1}, {0,-1}
    };

    // 根据 origin 推回自己的 chunk 坐标
    int cx = (int)origin.x / CHUNK_SIZE;
    int cz = (int)origin.z / CHUNK_SIZE;

    for(auto &d : dirs){
        int nx = cx + d.first;
        int nz = cz + d.second;

        auto it = gChunks.find({nx, nz});
        if(it == gChunks.end()) return false;          // 邻居还没分配
        if(!it->second->generated) return false;       // 邻居还没生成方块
    }
    return true;
}

// 只生成 Block 数据
void Chunk::generate() {
    blocks.clear();

    // 初始化 grid
    for (int x = 0; x < CHUNK_SIZE; x++)
        for (int z = 0; z < CHUNK_SIZE; z++)
            for (int y = 0; y < CHUNK_HEIGHT; y++)
                blockGrid[x][y][z] = nullptr;

    const int SEA_LEVEL = 8;

    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int z = 0; z < CHUNK_SIZE; z++) {
            int worldX = (int)origin.x + x;
            int worldZ = (int)origin.z + z;

            float h = noise.GetNoise((float)worldX, (float)worldZ); // [-1,1]
            int height = (int)((h + 1.0f) * 10.0f); // 映射到 0~20

            // 填充方块
            for (int y = -1; y < height; y++) {
                glm::vec3 pos = origin + glm::vec3(x, y, z);
                BlockType type;

                if (y == height - 1) {
                    // 顶层方块
                    if (height < SEA_LEVEL + 2) {
                        type = BlockType::Sand;   // 低处用沙子
                    } else {
                        type = BlockType::Grass;  // 高处用草
                    }
                } else {
                    type = BlockType::Stone; // 地下石头
                }

                auto block = std::make_shared<Block>(type);

                if (y >= 0 && y < CHUNK_HEIGHT)
                    blockGrid[x][y][z] = block;

                blocks.push_back({block, pos});
            }

            // 如果柱子低于海平面 -> 补水
            if (height < SEA_LEVEL) {
                for (int y = height; y < SEA_LEVEL; y++) {
                    glm::vec3 pos = origin + glm::vec3(x, y, z);
                    auto water = std::make_shared<Block>(BlockType::Water);
                    if (y >= 0 && y < CHUNK_HEIGHT)
                        blockGrid[x][y][z] = water;
                    blocks.push_back({water, pos});
                }
            }
        }
    }

    generated = true;
    dirty = true;
}

// 第二步：构建 Mesh（前提：邻居也生成了）
void Chunk::finalizeMesh() {
    updateFaceVisibility();
    buildMesh();
    dirty = false;
}


// 简单面剔除（仅检查同一 Chunk 内方块）
#include "Globals.h"

void Chunk::updateFaceVisibility() {
    for(auto &b : blocks) {
        glm::vec3 wp = b.position; // 世界坐标
        Block *block = b.block.get();

        block->visibleFaces[Face::TOP]    = getBlockAtGlobal(wp.x, wp.y + 1, wp.z) == nullptr;
        block->visibleFaces[Face::BOTTOM] = getBlockAtGlobal(wp.x, wp.y - 1, wp.z) == nullptr;
        block->visibleFaces[Face::LEFT]   = getBlockAtGlobal(wp.x - 1, wp.y, wp.z) == nullptr;
        block->visibleFaces[Face::RIGHT]  = getBlockAtGlobal(wp.x + 1, wp.y, wp.z) == nullptr;
        block->visibleFaces[Face::FRONT]  = getBlockAtGlobal(wp.x, wp.y, wp.z + 1) == nullptr;
        block->visibleFaces[Face::BACK]   = getBlockAtGlobal(wp.x, wp.y, wp.z - 1) == nullptr;
    }
}


// —— 新增：释放旧批次 —— //
void Chunk::clearMesh()
{
    for (auto &b : batches)
    {
        glDeleteVertexArrays(1, &b.vao);
        glDeleteBuffers(1, &b.vbo);
    }
    batches.clear();
}

// —— 新增：构建批次（按纹理ID分组） —— //
void Chunk::buildMesh()
{
    clearMesh();

    // 纹理ID -> 顶点数组
    std::unordered_map<GLuint, std::vector<float>> groups;
    groups.reserve(32);

    // 改造 emit：传 face 类型，内部决定顶点数据
    auto emit = [&](GLuint tex, const glm::vec3 &pos, Face face)
    {
        auto &buf = groups[tex];
        switch(face){
            case Face::TOP:    appendFace(buf, pos, FACE_TOP); break;
            case Face::BOTTOM: appendFace(buf, pos, FACE_BOTTOM); break;
            case Face::LEFT:   appendFace(buf, pos, FACE_LEFT); break;
            case Face::RIGHT:  appendFace(buf, pos, FACE_RIGHT); break;
            case Face::FRONT:  appendFace(buf, pos, FACE_FRONT); break;
            case Face::BACK:   appendFace(buf, pos, FACE_BACK); break;
        }
    };

    // 遍历所有可见面
    for (auto &inst : blocks)
    {
        Block &b = *inst.block.get();
        const glm::vec3 &pos = inst.position;

        if (b.visibleFaces[Face::TOP])
            emit(b.getTextureForFace(Face::TOP), pos, Face::TOP);
        if (b.visibleFaces[Face::BOTTOM])
            emit(b.getTextureForFace(Face::BOTTOM), pos, Face::BOTTOM);
        if (b.visibleFaces[Face::LEFT])
            emit(b.getTextureForFace(Face::LEFT), pos, Face::LEFT);
        if (b.visibleFaces[Face::RIGHT])
            emit(b.getTextureForFace(Face::RIGHT), pos, Face::RIGHT);
        if (b.visibleFaces[Face::FRONT])
            emit(b.getTextureForFace(Face::FRONT), pos, Face::FRONT);
        if (b.visibleFaces[Face::BACK])
            emit(b.getTextureForFace(Face::BACK), pos, Face::BACK);
    }

    // 为每个分组创建 VAO/VBO
    batches.reserve(groups.size());
    for (auto &kv : groups)
    {
        GLuint tex = kv.first;
        std::vector<float> &data = kv.second;
        if (data.empty()) continue;

        DrawBatch batch;
        batch.texture = tex;
        batch.vertexCount = static_cast<GLsizei>(data.size() / 5);

        glGenVertexArrays(1, &batch.vao);
        glGenBuffers(1, &batch.vbo);
        glBindVertexArray(batch.vao);
        glBindBuffer(GL_ARRAY_BUFFER, batch.vbo);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
        batches.push_back(batch);
    }
}

// —— 改：渲染改为绘制批次 —— //
void Chunk::render(Shader &shader)
{

    shader.use();
    // 注意：我们在顶点里已经写入了世界坐标，这里 model 用单位矩阵
    glm::mat4 model(1.0f);
    shader.setMat4("model", model);

    for (auto &b : batches)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, b.texture);
        shader.setInt("texture1", 0);

        glBindVertexArray(b.vao);
        glDrawArrays(GL_TRIANGLES, 0, b.vertexCount);
    }
    glBindVertexArray(0);
}
Block* Chunk::getBlockAtLocal(int lx, int y, int lz) {
    if(lx < 0 || lx >= CHUNK_SIZE || lz < 0 || lz >= CHUNK_SIZE || y < 0 || y >= CHUNK_HEIGHT)
        return nullptr;
    auto &ptr = blockGrid[lx][y][lz];
    return ptr ? ptr.get() : nullptr;
}

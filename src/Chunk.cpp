#include "Chunk.h"
#include <gtc/matrix_transform.hpp>
#include <unordered_map>

// ===== 面模板（与 Block::initSharedMesh 的顺序/UV一致） =====
const Chunk::Vtx Chunk::FACE_TOP[6] = {
    {-0.5f, 0.5f, -0.5f, 0.0f, 1.0f},
    {0.5f, 0.5f, -0.5f, 1.0f, 1.0f},
    {0.5f, 0.5f, 0.5f, 1.0f, 0.0f},
    {0.5f, 0.5f, 0.5f, 1.0f, 0.0f},
    {-0.5f, 0.5f, 0.5f, 0.0f, 0.0f},
    {-0.5f, 0.5f, -0.5f, 0.0f, 1.0f},
};
const Chunk::Vtx Chunk::FACE_BOTTOM[6] = {
    {-0.5f, -0.5f, -0.5f, 0.0f, 1.0f},
    {0.5f, -0.5f, -0.5f, 1.0f, 1.0f},
    {0.5f, -0.5f, 0.5f, 1.0f, 0.0f},
    {0.5f, -0.5f, 0.5f, 1.0f, 0.0f},
    {-0.5f, -0.5f, 0.5f, 0.0f, 0.0f},
    {-0.5f, -0.5f, -0.5f, 0.0f, 1.0f},
};
const Chunk::Vtx Chunk::FACE_LEFT[6] = {
    {-0.5f, -0.5f, -0.5f, 1.0f, 1.0f},
    {-0.5f, 0.5f, -0.5f, 1.0f, 0.0f},
    {-0.5f, 0.5f, 0.5f, 0.0f, 0.0f},
    {-0.5f, 0.5f, 0.5f, 0.0f, 0.0f},
    {-0.5f, -0.5f, 0.5f, 0.0f, 1.0f},
    {-0.5f, -0.5f, -0.5f, 1.0f, 1.0f},
};
const Chunk::Vtx Chunk::FACE_RIGHT[6] = {
    {0.5f, -0.5f, -0.5f, 0.0f, 1.0f},
    {0.5f, 0.5f, -0.5f, 0.0f, 0.0f},
    {0.5f, 0.5f, 0.5f, 1.0f, 0.0f},
    {0.5f, 0.5f, 0.5f, 1.0f, 0.0f},
    {0.5f, -0.5f, 0.5f, 1.0f, 1.0f},
    {0.5f, -0.5f, -0.5f, 0.0f, 1.0f},
};
const Chunk::Vtx Chunk::FACE_FRONT[6] = {
    {-0.5f, -0.5f, 0.5f, 0.0f, 1.0f},
    {0.5f, -0.5f, 0.5f, 1.0f, 1.0f},
    {0.5f, 0.5f, 0.5f, 1.0f, 0.0f},
    {0.5f, 0.5f, 0.5f, 1.0f, 0.0f},
    {-0.5f, 0.5f, 0.5f, 0.0f, 0.0f},
    {-0.5f, -0.5f, 0.5f, 0.0f, 1.0f},
};
const Chunk::Vtx Chunk::FACE_BACK[6] = {
    {-0.5f, -0.5f, -0.5f, 0.0f, 1.0f},
    {0.5f, -0.5f, -0.5f, 1.0f, 1.0f},
    {0.5f, 0.5f, -0.5f, 1.0f, 0.0f},
    {0.5f, 0.5f, -0.5f, 1.0f, 0.0f},
    {-0.5f, 0.5f, -0.5f, 0.0f, 0.0f},
    {-0.5f, -0.5f, -0.5f, 0.0f, 1.0f},
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

void Chunk::generate()
{
    blocks.clear();

    // 生成底层石头
    for (int x = 0; x < CHUNK_SIZE; x++)
    {
        for (int z = 0; z < CHUNK_SIZE; z++)
        {
            glm::vec3 pos = origin + glm::vec3(x, -1, z);
            auto grassBlock = std::make_shared<Block>(BlockType::Stone);
            blocks.push_back({grassBlock, pos});
        }
    }

    // 生成草方块层
    for (int x = 0; x < CHUNK_SIZE; x++)
    {
        for (int z = 0; z < CHUNK_SIZE; z++)
        {
            glm::vec3 pos = origin + glm::vec3(x, 0, z);
            auto grassBlock = std::make_shared<Block>(BlockType::Grass);
            blocks.push_back({grassBlock, pos});
        }
    }

    updateFaceVisibility();
    buildMesh(); // —— 新增：生成合并网格 —— //
    dirty = false;
}

// 简单面剔除（仅检查同一 Chunk 内方块）
void Chunk::updateFaceVisibility()
{
    auto getBlockAtLocal = [&](int lx, int y, int lz) -> Block *
    {
        for (auto &b : blocks)
        {
            glm::vec3 p = b.position - origin; // 转为局部坐标
            if ((int)p.x == lx && (int)p.y == y && (int)p.z == lz)
                return b.block.get();
        }
        return nullptr;
    };

    for (auto &b : blocks)
    {
        glm::vec3 lp = b.position - origin; // 局部坐标
        Block *block = b.block.get();

        block->visibleFaces[Face::TOP] = getBlockAtLocal(lp.x, lp.y + 1, lp.z) == nullptr;
        block->visibleFaces[Face::BOTTOM] = getBlockAtLocal(lp.x, lp.y - 1, lp.z) == nullptr;
        block->visibleFaces[Face::LEFT] = getBlockAtLocal(lp.x - 1, lp.y, lp.z) == nullptr;
        block->visibleFaces[Face::RIGHT] = getBlockAtLocal(lp.x + 1, lp.y, lp.z) == nullptr;
        block->visibleFaces[Face::FRONT] = getBlockAtLocal(lp.x, lp.y, lp.z + 1) == nullptr;
        block->visibleFaces[Face::BACK] = getBlockAtLocal(lp.x, lp.y, lp.z - 1) == nullptr;
    }
    dirty = true; // 网格需要重建
}

// —— 新增：释放旧批次 —— //
void Chunk::clearMesh()
{
    for (auto &b : batches)
    {
        if (b.vao)
            glDeleteVertexArrays(1, &b.vao);
        if (b.vbo)
            glDeleteBuffers(1, &b.vbo);
    }
    batches.clear();
}

// —— 新增：构建批次（按纹理ID分组） —— //
void Chunk::buildMesh()
{
    clearMesh();

    // 纹理ID -> 顶点数组(非索引): 每顶点5个float (x,y,z,u,v)
    std::unordered_map<GLuint, std::vector<float>> groups;
    groups.reserve(32);

    auto emit = [&](GLuint tex, const glm::vec3 &pos, const Vtx face[6])
    {
        auto &buf = groups[tex];
        appendFace(buf, pos, face);
    };

    // 遍历所有可见面，按纹理加入对应分组
    for (auto &inst : blocks)
    {
        Block &b = *inst.block.get();
        const glm::vec3 &pos = inst.position;

        if (b.visibleFaces[Face::TOP])
            emit(b.getTextureForFace(Face::TOP), pos, FACE_TOP);
        if (b.visibleFaces[Face::BOTTOM])
            emit(b.getTextureForFace(Face::BOTTOM), pos, FACE_BOTTOM);
        if (b.visibleFaces[Face::LEFT])
            emit(b.getTextureForFace(Face::LEFT), pos, FACE_LEFT);
        if (b.visibleFaces[Face::RIGHT])
            emit(b.getTextureForFace(Face::RIGHT), pos, FACE_RIGHT);
        if (b.visibleFaces[Face::FRONT])
            emit(b.getTextureForFace(Face::FRONT), pos, FACE_FRONT);
        if (b.visibleFaces[Face::BACK])
            emit(b.getTextureForFace(Face::BACK), pos, FACE_BACK);
    }

    // 为每个分组创建 VAO/VBO
    batches.reserve(groups.size());
    for (auto &kv : groups)
    {
        GLuint tex = kv.first;
        std::vector<float> &data = kv.second;
        if (data.empty())
            continue;

        DrawBatch batch;
        batch.texture = tex;
        batch.vertexCount = static_cast<GLsizei>(data.size() / 5);

        glGenVertexArrays(1, &batch.vao);
        glGenBuffers(1, &batch.vbo);
        glBindVertexArray(batch.vao);
        glBindBuffer(GL_ARRAY_BUFFER, batch.vbo);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);

        // layout(location=0) vec3 aPos
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);
        // layout(location=1) vec2 aTexCoord
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
        batches.push_back(batch);
    }
    dirty = false;
}

// —— 改：渲染改为绘制批次 —— //
void Chunk::render(Shader &shader)
{
    if (dirty)
        buildMesh();

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

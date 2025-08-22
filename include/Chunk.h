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
    // —— 新增 —— //
    void buildMesh();   // 构建批处理网格
    void clearMesh();   // 释放VAO/VBO
    ~Chunk() { clearMesh(); }

    void generate();
    void finalizeMesh(); // 构建 Mesh（前提：邻居也生成了）
    void render(Shader& shader);
    glm::vec3 origin;
    std::vector<BlockInstance> blocks;
    Block* getBlockAtLocal(int lx, int y, int lz);
    bool neighborsReady();

    bool generated = false; // 标记方块是否已生成
    bool dirty = true; // 有变化需要重建mesh

private:
    


    void updateFaceVisibility();

    // —— 新增：按纹理分组的绘制批次 —— //
    struct DrawBatch {
        GLuint vao = 0;
        GLuint vbo = 0;
        GLuint texture = 0;     // 使用的纹理ID
        GLsizei vertexCount = 0; // 顶点总数（非索引，pos+uv）
    };
    std::vector<DrawBatch> batches;

    // —— 新增：生成各个面的顶点工具 —— //
    struct Vtx { float x,y,z,u,v; };
    static void appendFace(std::vector<float>& out, const glm::vec3& base, const Vtx face[6]);

    // 六个面的局部模板（以方块中心为(0,0,0)，边长1）
    static const Vtx FACE_TOP[6];
    static const Vtx FACE_BOTTOM[6];
    static const Vtx FACE_LEFT[6];
    static const Vtx FACE_RIGHT[6];
    static const Vtx FACE_FRONT[6];
    static const Vtx FACE_BACK[6];
};

#endif

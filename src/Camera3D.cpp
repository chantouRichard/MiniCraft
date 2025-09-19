#include "Camera3D.h"
#include <iostream>
#include <unordered_map>
#include "Globals.h"

using namespace std;

Camera3D::Camera3D(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : Front(glm::vec3(0.0f, 0.0f, 1.0f)),
      MovementSpeed(5.0f),
      MouseSensitivity(0.1f),
      Zoom(45.0f)
{
    Position = position;
    WorldUp = up;
    Yaw = yaw;
    Pitch = pitch;
    updateCameraVectors();
}

glm::mat4 Camera3D::GetViewMatrix()
{
    // 摄像头在玩家头顶
    glm::vec3 cameraPos = Position + glm::vec3(0, height, 0);
    return glm::lookAt(cameraPos, cameraPos + Front, Up);
}

bool Camera3D::IsColliding(float x, float y, float z)
{
    int minX = (int)floor(x - radius);
    int maxX = (int)floor(x + radius);
    int minZ = (int)floor(z - radius);
    int maxZ = (int)floor(z + radius);
    int minY = (int)floor(y);
    int maxY = (int)floor(y + height);

    for (int xi = minX; xi <= maxX; xi++)
    {
        for (int yi = minY; yi <= maxY; yi++)
        {
            for (int zi = minZ; zi <= maxZ; zi++)
            {
                int cx = (int)floor((float)xi / CHUNK_SIZE);
                int cz = (int)floor((float)zi / CHUNK_SIZE);
                auto it = gChunks.find({cx, cz});

                if (it != gChunks.end())
                {
                    Chunk *chunk = it->second.get();
                    // 遍历chunk中方块
                    int lx = xi - cx * CHUNK_SIZE;
                    int lz = zi - cz * CHUNK_SIZE;
                    if (yi >= 0 && yi < CHUNK_HEIGHT)
                    {
                        Block *b = chunk->blockGrid[lx][yi][lz].get();
                        if (b && b->type != BlockType::Air)
                        {
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}

void Camera3D::MoveWithCollision(const glm::vec3 &move, float deltaTime, Camera_Movement direction)
{
    if (direction != Camera_Movement::UP)
    {
        glm::vec3 newPos = Position + glm::vec3(move.x, 0.0f, move.z);

        // --------- XZ 平面碰撞检测 ---------
        if (!IsColliding(newPos.x, Position.y, newPos.z))
        {
            Position.x = newPos.x;
            Position.z = newPos.z;
        }

        // --------- Y 轴物理更新（始终执行） ---------
        velocityY += gravity * deltaTime;
        float newY = Position.y + velocityY * deltaTime;

        if (IsColliding(Position.x, newY, Position.z) || newY <= -10.0f)
        {
            onGround = true;
            velocityY = 0;

            // 将人物放到方块上方
            Position.y = floor(Position.y);
        }
        else
        {
            onGround = false;
        }
    }
    else
    {
        glm::vec3 newPos = Position + glm::vec3(move.x, 0.0f, move.z);

        // --------- XZ 平面碰撞检测 ---------
        if (!IsColliding(newPos.x, Position.y, newPos.z))
        {
            Position.x = newPos.x;
            Position.z = newPos.z;
        }

        // --------- Y 轴物理更新（始终执行） ---------
        velocityY += gravity * deltaTime;
        float newY = Position.y + velocityY * deltaTime;

        if (IsColliding(Position.x, newY, Position.z) || newY <= -10.0f)
        {
            onGround = true;
            velocityY = 0;

            // 将人物放到方块上方
            Position.y = floor(Position.y);
        }
        else
        {
            Position.y = newY;
            onGround = false;
        }
    }
}
glm::vec3 pendingMove = glm::vec3(0.0f, 0.0f, 0.0f);
void Camera3D::ProcessKeyboard(Camera_Movement direction, float deltaTime)
{
    float velocity = MovementSpeed * deltaTime;

    glm::vec3 horizontalFront = glm::normalize(glm::vec3(Front.x, 0.0f, Front.z));
    glm::vec3 horizontalRight = glm::normalize(glm::vec3(Right.x, 0.0f, Right.z));

    glm::vec3 move(0.0f);

    if (direction == FORWARD)
        move += horizontalFront * velocity;
    if (direction == BACKWARD)
        move -= horizontalFront * velocity;
    if (direction == LEFTWARD)
        move -= horizontalRight * velocity;
    if (direction == RIGHTWARD)
        move += horizontalRight * velocity;

    if (mode == CameraMode::FLIGHT)
    {
        if (direction == UP)
            move.y += velocity;
        if (direction == DOWN)
            move.y -= velocity;
    }
    if (mode == CameraMode::SURVIVAL && direction == UP && onGround)
    {
        velocityY = jumpSpeed; // 跳跃只修改竖直速度
    }

    // 存下 move 给 UpdatePhysics 使用
    pendingMove += move;
}
void Camera3D::UpdatePhysics(float deltaTime)
{
    if (mode == CameraMode::SURVIVAL)
    {
        // --------- 水平移动 ---------
        glm::vec3 newPos = Position + glm::vec3(pendingMove.x, 0.0f, pendingMove.z);
        if (!IsColliding(newPos.x, Position.y, newPos.z))
        {
            Position.x = newPos.x;
            Position.z = newPos.z;
        }

        // --------- 垂直物理 ---------
        velocityY += gravity * deltaTime;
        float newY = Position.y + velocityY * deltaTime;

        if (IsColliding(Position.x, newY, Position.z) || newY <= -10.0f)
        {
            onGround = true;
            velocityY = 0;
            Position.y = floor(Position.y);
        }
        else
        {
            Position.y = newY;
            onGround = false;
        }
    }
    else if (mode == CameraMode::FLIGHT)
    {
        Position += pendingMove;
    }

    // 清空，避免重复叠加
    pendingMove = glm::vec3(0.0f);
}

void Camera3D::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch)
{
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    Yaw += xoffset;
    Pitch += yoffset;

    if (constrainPitch)
    {
        if (Pitch > 89.0f)
            Pitch = 89.0f;
        if (Pitch < -89.0f)
            Pitch = -89.0f;
    }

    updateCameraVectors();
}

void Camera3D::ProcessMouseScroll(float yoffset)
{
    Zoom -= (float)yoffset;
    if (Zoom < 1.0f)
        Zoom = 1.0f;
    if (Zoom > 45.0f)
        Zoom = 45.0f;
}

void Camera3D::updateCameraVectors()
{
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);

    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up = glm::normalize(glm::cross(Right, Front));
}

// 破坏、放置方块
#include <algorithm>
#include <chrono>
std::chrono::steady_clock::time_point lastBreakTime = std::chrono::steady_clock::now();
float breakCooldown = 0.2f; // 秒

RaycastHit Camera3D::Raycast(float maxDistance)
{
    glm::vec3 ray = glm::normalize(Front);
    glm::vec3 pos = Position + glm::vec3(0.0f, height, 0.0f);

    int bx = floor(pos.x);
    int by = floor(pos.y);
    int bz = floor(pos.z);

    float stepX = (ray.x > 0) ? 1.0f : -1.0f;
    float stepY = (ray.y > 0) ? 1.0f : -1.0f;
    float stepZ = (ray.z > 0) ? 1.0f : -1.0f;

    float tMaxX = (ray.x != 0.0f) ? ((stepX > 0 ? (bx + 1.0f - pos.x) : (pos.x - bx)) / fabs(ray.x)) : INFINITY;
    float tMaxY = (ray.y != 0.0f) ? ((stepY > 0 ? (by + 1.0f - pos.y) : (pos.y - by)) / fabs(ray.y)) : INFINITY;
    float tMaxZ = (ray.z != 0.0f) ? ((stepZ > 0 ? (bz + 1.0f - pos.z) : (pos.z - bz)) / fabs(ray.z)) : INFINITY;

    float tDeltaX = (ray.x != 0.0f) ? (1.0f / fabs(ray.x)) : INFINITY;
    float tDeltaY = (ray.y != 0.0f) ? (1.0f / fabs(ray.y)) : INFINITY;
    float tDeltaZ = (ray.z != 0.0f) ? (1.0f / fabs(ray.z)) : INFINITY;

    float t = 0.0f;

    while (t < maxDistance)
    {
        int cx = floor((float)bx / CHUNK_SIZE);
        int cz = floor((float)bz / CHUNK_SIZE);
        auto it = gChunks.find({cx, cz});
        if (it != gChunks.end())
        {
            auto chunk = it->second;
            int lx = bx - cx * CHUNK_SIZE;
            int lz = bz - cz * CHUNK_SIZE;

            if (by >= 0 && by < CHUNK_HEIGHT)
            {
                Block *target = chunk->getBlockAtLocal(lx, by, lz);
                if (target)
                {
                    Face hitFace;

                    // 判断是哪一条轴最后更新的 t
                    if (tMaxX - tDeltaX < tMaxY - tDeltaY && tMaxX - tDeltaX < tMaxZ - tDeltaZ)
                        hitFace = (stepX > 0) ? LEFT : RIGHT; // 注意，沿X正方向走意味着击中了方块的左面
                    else if (tMaxY - tDeltaY < tMaxZ - tDeltaZ)
                        hitFace = (stepY > 0) ? BOTTOM : TOP; // Y正方向意味着击中方块底面
                    else
                        hitFace = (stepZ > 0) ? BACK : FRONT; // Z正方向意味着击中方块后面

                    return {true, bx, by, bz, target, chunk, hitFace};
                }
            }
        }

        if (tMaxX < tMaxY)
        {
            if (tMaxX < tMaxZ)
            {
                bx += (int)stepX;
                t = tMaxX;
                tMaxX += tDeltaX;
            }
            else
            {
                bz += (int)stepZ;
                t = tMaxZ;
                tMaxZ += tDeltaZ;
            }
        }
        else
        {
            if (tMaxY < tMaxZ)
            {
                by += (int)stepY;
                t = tMaxY;
                tMaxY += tDeltaY;
            }
            else
            {
                bz += (int)stepZ;
                t = tMaxZ;
                tMaxZ += tDeltaZ;
            }
        }
    }
    return {false, 0, 0, 0, nullptr, nullptr};
}

void Camera3D::RaycastAndBreakBlock()
{
    auto now = std::chrono::steady_clock::now();
    float elapsed = std::chrono::duration<float>(now - lastBreakTime).count();
    if (elapsed < breakCooldown)
        return;
    lastBreakTime = now;

    RaycastHit hit = Raycast(5.0f);
    if (!hit.hit)
        return;

    int bx = hit.bx, by = hit.by, bz = hit.bz;
    auto chunk = hit.chunk;

    int cx = (int)floor((float)bx / CHUNK_SIZE);
    int cz = (int)floor((float)bz / CHUNK_SIZE);
    int lx = bx - cx * CHUNK_SIZE;
    int lz = bz - cz * CHUNK_SIZE;

    // 删除 blockGrid
    chunk->blockGrid[lx][by][lz] = nullptr;

    // 删除 blocks 中的实例
    chunk->blocks.erase(
        std::remove_if(chunk->blocks.begin(), chunk->blocks.end(),
                       [&](const BlockInstance &inst)
                       {
                           return (int)inst.position.x == bx &&
                                  (int)inst.position.y == by &&
                                  (int)inst.position.z == bz;
                       }),
        chunk->blocks.end());

    // 标记当前区块需要重建
    chunk->dirty = true;

    // —— 新增：检查是否是边缘方块 —— //
    if (lx == 0) {
        auto it = gChunks.find({cx - 1, cz});
        if (it != gChunks.end()) it->second->dirty = true;
    }
    if (lx == CHUNK_SIZE - 1) {
        auto it = gChunks.find({cx + 1, cz});
        if (it != gChunks.end()) it->second->dirty = true;
    }
    if (lz == 0) {
        auto it = gChunks.find({cx, cz - 1});
        if (it != gChunks.end()) it->second->dirty = true;
    }
    if (lz == CHUNK_SIZE - 1) {
        auto it = gChunks.find({cx, cz + 1});
        if (it != gChunks.end()) it->second->dirty = true;
    }
}

const char* FaceToString(Face f) {
    switch(f) {
        case TOP: return "TOP";
        case BOTTOM: return "BOTTOM";
        case LEFT: return "LEFT";
        case RIGHT: return "RIGHT";
        case FRONT: return "FRONT";
        case BACK: return "BACK";
        default: return "UNKNOWN";
    }
}
void Camera3D::RaycastAndPlaceBlock()
{
    auto now = std::chrono::steady_clock::now();
    float elapsed = std::chrono::duration<float>(now - lastBreakTime).count();
    if (elapsed < breakCooldown)
        return;
    lastBreakTime = now;

    // 1. 射线检测
    RaycastHit hit = Raycast(5.0f);
    if (!hit.hit)
        return; // 没有击中任何方块

    // 2. 计算放置位置
    glm::ivec3 placePos = {hit.bx, hit.by, hit.bz};

    // cout<<"面: "<<FaceToString(hit.face)<<endl;
    // 用射线方向判断要放置在哪一侧
    glm::vec3 ray = glm::normalize(Front);
    // 根据射线方向选择要放置的邻近方块
    if (fabs(ray.x) > fabs(ray.y) && fabs(ray.x) > fabs(ray.z))
        placePos.x += (ray.x > 0) ? 1 : -1;
    else if (fabs(ray.y) > fabs(ray.z))
        placePos.y += (ray.y > 0) ? 1 : -1;
    else
        placePos.z += (ray.z > 0) ? 1 : -1;

    // 3. 找到对应 Chunk
    int cx = floor((float)placePos.x / CHUNK_SIZE);
    int cz = floor((float)placePos.z / CHUNK_SIZE);
    auto it = gChunks.find({cx, cz});
    if (it == gChunks.end())
        return; // Chunk 不存在

    auto chunk = it->second;

    // 4. 转换为 Chunk 内局部坐标
    int lx = placePos.x - cx * CHUNK_SIZE;
    int ly = placePos.y;
    int lz = placePos.z - cz * CHUNK_SIZE;

    if (ly < 0 || ly >= CHUNK_HEIGHT)
        return; // 超出高度限制
    if (chunk->blockGrid[lx][ly][lz] != nullptr)
    {
        cout << "不能覆盖已有方块: " << chunk.get()->origin.x << " " << chunk.get()->origin.y << " " << chunk.get()->origin.z << endl;
        return;
    } // 不能覆盖已有方块

    // 5. 创建新方块
    auto newBlock = std::make_shared<Block>(BlockType::Stone);
    chunk->blockGrid[lx][ly][lz] = newBlock;
    chunk->blocks.push_back({newBlock, glm::vec3(placePos)});

    // 7. 标记 Chunk 重建 Mesh
    chunk->dirty = true;
    chunk->finalizeMesh();
}

void Camera3D::drawSelectedBlockHighlight(Shader &shader)
{
    RaycastHit hit = Raycast(5.0f);
    if (!hit.hit)
        return;

    glm::vec3 blockPos(hit.bx, hit.by, hit.bz);

    float size = 1.0f;
    float vertices[] = {
        blockPos.x,
        blockPos.y,
        blockPos.z,
        blockPos.x + size,
        blockPos.y,
        blockPos.z,
        blockPos.x + size,
        blockPos.y,
        blockPos.z + size,
        blockPos.x,
        blockPos.y,
        blockPos.z + size,
        blockPos.x,
        blockPos.y + size,
        blockPos.z,
        blockPos.x + size,
        blockPos.y + size,
        blockPos.z,
        blockPos.x + size,
        blockPos.y + size,
        blockPos.z + size,
        blockPos.x,
        blockPos.y + size,
        blockPos.z + size,
    };

    unsigned int indices[] = {
        0, 1, 1, 2, 2, 3, 3, 0,
        4, 5, 5, 6, 6, 7, 7, 4,
        0, 4, 1, 5, 2, 6, 3, 7};

    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    shader.use();
    glBindVertexArray(VAO);
    glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);

    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);
}
// 调试函数
void Camera3D::debugPrintBlockBelow()
{
    // 当前相机位置
    int bx = (int)std::floor(Position.x);
    int by = (int)std::floor(Position.y) - 1; // 往下一个方块
    int bz = (int)std::floor(Position.z);

    // 检查高度范围
    if (by < 0 || by >= CHUNK_HEIGHT)
    {
        std::cout << "[Debug] Block below out of world range." << std::endl;
        return;
    }

    // 定位 chunk
    int cx = (int)std::floor((float)bx / CHUNK_SIZE);
    int cz = (int)std::floor((float)bz / CHUNK_SIZE);

    auto it = gChunks.find({cx, cz});
    if (it == gChunks.end())
    {
        std::cout << "[Debug] No chunk found at (" << cx << ", " << cz << ")" << std::endl;
        return;
    }

    Chunk *chunk = it->second.get();

    int lx = bx - cx * CHUNK_SIZE;
    int lz = bz - cz * CHUNK_SIZE;

    Block *b = chunk->blockGrid[lx][by][lz].get();
    if (!b)
    {
        std::cout << "[Debug] No block found below at (" << bx << "," << by << "," << bz << ")" << std::endl;
        return;
    }

    if (b->type == BlockType::Air)
    {
        std::cout << "[Debug] Block below is Air." << std::endl;
        return;
    }

    // 打印方块纹理信息
    std::cout << "[Debug] Block below at (" << bx << "," << by << "," << bz << "): type=" << (int)b->type << std::endl;

    GLuint texTop = b->getTextureForFace(Face::TOP);
    GLuint texBottom = b->getTextureForFace(Face::BOTTOM);
    GLuint texSide = b->getTextureForFace(Face::FRONT);

    std::cout << "   Top texture ID:    " << texTop << (texTop ? " (loaded)" : " (not loaded)");
    std::cout << "   Bottom texture ID: " << texBottom << (texBottom ? " (loaded)" : " (not loaded)");
    std::cout << "   Side texture ID:   " << texSide << (texSide ? " (loaded)" : " (not loaded)");

    std::cout << "   Visible faces:     " << (b->visibleFaces[Face::TOP] ? "T" : "-") << (b->visibleFaces[Face::BOTTOM] ? "B" : "-") << (b->visibleFaces[Face::LEFT] ? "L" : "-") << (b->visibleFaces[Face::RIGHT] ? "R" : "-") << (b->visibleFaces[Face::FRONT] ? "F" : "-") << (b->visibleFaces[Face::BACK] ? "B" : "-") << std::endl;

}
#ifndef CAMERA3D_H
#define CAMERA3D_H

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include "Shader.h"
#include <memory>
#include "Block.h"
#include "Chunk.h"

enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFTWARD,
    RIGHTWARD,
    UP,
    DOWN
};

enum class CameraMode {
    SURVIVAL,  // 受限移动，不能随意上下
    FLIGHT     // 自由飞行
};


struct RaycastHit {
    bool hit;
    int bx, by, bz;  // 命中的方块世界坐标
    Block* block;
    std::shared_ptr<Chunk> chunk;
    Face face;
};

class Camera3D {
public:
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    float Yaw;
    float Pitch;

    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    Camera3D(glm::vec3 position = glm::vec3(0.0f, 23.0f, 1.0f),
             glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
             float yaw = 90.0f,
             float pitch = 0.0f);

    glm::mat4 GetViewMatrix();
    void ProcessKeyboard(Camera_Movement direction, float deltaTime);
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
    void ProcessMouseScroll(float yoffset);

    CameraMode mode = CameraMode::SURVIVAL;  // 默认生存模式

    // 是否在地面上
    bool onGround = false;

    // 垂直速度，用于重力和跳跃
    float velocityY = 0.0f;

    // 重力加速度（负数向下）
    const float gravity = -30.0f;

    // 跳跃速度
    const float jumpSpeed = 9.0f;

    // 玩家碰撞盒高度
    const float height = 1.8f;
    const float radius = 0.3f;  // 玩家碰撞半径

    bool IsColliding(float x, float y, float z);
    void MoveWithCollision(const glm::vec3& move, float deltaTime, Camera_Movement direction);

    // 破坏、放置方块
    void RaycastAndBreakBlock();
    void RaycastAndPlaceBlock();

    void drawSelectedBlockHighlight(Shader& shader);
    void UpdatePhysics(float deltaTime);

    RaycastHit Raycast(float maxDistance);

    private:
        void updateCameraVectors();
};

#endif

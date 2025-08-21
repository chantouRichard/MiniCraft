#include "Camera.h"

Camera2D::Camera2D(float startX, float startY, float startZoom)
    : x(startX), y(startY), zoom(startZoom) {}

void Camera2D::ProcessKeyboard(char direction, float deltaTime) {
    float speed = 200.0f; // 每秒移动像素数

    // if (direction == 'W') zoom += 0.5f * deltaTime;   // 前：放大
    // if (direction == 'S') zoom -= 0.5f * deltaTime;   // 后：缩小
    if (direction == 'W') y -= speed * deltaTime;     // 左
    if (direction == 'S') y += speed * deltaTime;     // 右
    if (direction == 'A') x -= speed * deltaTime;     // 左
    if (direction == 'D') x += speed * deltaTime;     // 右

    if (zoom < 0.1f) zoom = 0.1f;  // 避免缩小到负值或太小
}

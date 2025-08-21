#include "Camera3D.h"
#include <iostream>

using namespace std;

Camera3D::Camera3D(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : Front(glm::vec3(0.0f, 0.0f, -1.0f)),
      MovementSpeed(5.0f),
      MouseSensitivity(0.1f),
      Zoom(45.0f) {
    Position = position;
    WorldUp = up;
    Yaw = yaw;
    Pitch = pitch;
    updateCameraVectors();
}

glm::mat4 Camera3D::GetViewMatrix() {
    return glm::lookAt(Position, Position + Front, Up);
}

void Camera3D::ProcessKeyboard(Camera_Movement direction, float deltaTime) {
    float velocity = MovementSpeed * deltaTime;

    // 水平移动
    glm::vec3 horizontalFront = glm::normalize(glm::vec3(Front.x, 0.0f, Front.z));
    glm::vec3 horizontalRight = glm::normalize(glm::vec3(Right.x, 0.0f, Right.z));

    if (direction == FORWARD)
        Position += horizontalFront * velocity;
    if (direction == BACKWARD)
        Position -= horizontalFront * velocity;
    if (direction == LEFT)
        Position -= horizontalRight * velocity;
    if (direction == RIGHT)
        Position += horizontalRight * velocity;

    // 垂直移动
    if (direction == UP)        // 空格
        Position.y += velocity;
    if (direction == DOWN)      // Shift
        Position.y -= velocity;
}


void Camera3D::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch) {
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    Yaw   += xoffset;
    Pitch += yoffset;

    if (constrainPitch) {
        if (Pitch > 89.0f) Pitch = 89.0f;
        if (Pitch < -89.0f) Pitch = -89.0f;
    }

    updateCameraVectors();
}

void Camera3D::ProcessMouseScroll(float yoffset) {
    Zoom -= (float)yoffset;
    if (Zoom < 1.0f) Zoom = 1.0f;
    if (Zoom > 45.0f) Zoom = 45.0f;
}

void Camera3D::updateCameraVectors() {
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);

    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up    = glm::normalize(glm::cross(Right, Front));
}

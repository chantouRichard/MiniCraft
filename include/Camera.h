#pragma once
#include <SDL2/SDL.h>

class Camera2D {
public:
    float x, y;     // 相机中心位置
    float zoom;     // 缩放比例

    Camera2D(float startX = 0.0f, float startY = 0.0f, float startZoom = 1.0f);

    void ProcessKeyboard(char direction, float deltaTime);
};

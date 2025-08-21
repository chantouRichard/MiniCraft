#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <iostream>
#include "Camera.h"

using namespace std;

Camera2D camera(0, 0, 1.0f);

// 游戏初始化
bool init(SDL_Window*& window, SDL_Renderer*& renderer) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        cout << "SDL_Init Error: " << SDL_GetError() << endl;
        return false;
    }

    window = SDL_CreateWindow("MiniCraft", 100, 100, 1200, 900, SDL_WINDOW_SHOWN);
    if (!window) {
        cout << "SDL_CreateWindow Error: " << SDL_GetError() << endl;
        SDL_Quit();
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        cout << "SDL_CreateRenderer Error: " << SDL_GetError() << endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return false;
    }

    return true;
}

// 游戏事件处理
void handleEvents(bool& running) {
    SDL_Event event;
    const Uint8* state = SDL_GetKeyboardState(NULL);

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            running = false;
        }
    }

    float deltaTime = 1.0f / 60.0f; // 简单固定帧率

    if (state[SDL_SCANCODE_W]) camera.ProcessKeyboard('W', deltaTime);
    if (state[SDL_SCANCODE_S]) camera.ProcessKeyboard('S', deltaTime);
    if (state[SDL_SCANCODE_A]) camera.ProcessKeyboard('A', deltaTime);
    if (state[SDL_SCANCODE_D]) camera.ProcessKeyboard('D', deltaTime);
}

// 游戏逻辑更新
void update(float deltaTime) {
    const Uint8* keyState = SDL_GetKeyboardState(NULL);
}

// 游戏渲染
void render(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    // 蓝色方块
    SDL_Rect blueSquare;
    blueSquare.w = 50 * camera.zoom;
    blueSquare.h = 50 * camera.zoom;
    blueSquare.x = (100 - camera.x) * camera.zoom + 400; // 400 = 屏幕中心
    blueSquare.y = (100 - camera.y) * camera.zoom + 300; // 300 = 屏幕中心

    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    SDL_RenderFillRect(renderer, &blueSquare);

    SDL_RenderPresent(renderer);
}


int main(int argc, char* argv[]) {
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    if (!init(window, renderer)) {
        return 1;
    }

    bool running = true;
    const int FPS = 60;
    const int frameDelay = 1000 / FPS;

    while (running) {
        Uint32 frameStart = SDL_GetTicks();

        handleEvents(running);
        float deltaTime = frameDelay / 1000.0f;  // 秒
        update(deltaTime);
        render(renderer);

        Uint32 frameTime = SDL_GetTicks() - frameStart;
        if (frameDelay > frameTime) {
            SDL_Delay(frameDelay - frameTime);
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

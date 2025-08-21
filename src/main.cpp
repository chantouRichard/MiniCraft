#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <gl.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include "Shader.h"
#include "Camera3D.h"

#include <iostream>
#include <chrono>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
unsigned int textureTop, textureSide, textureBottom;
int width, height, nrChannels;
unsigned char *data;
// 屏幕大小
const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 800;

// 相机
Camera3D camera;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// 时间
float deltaTime = 0.0f;
float lastFrame = 0.0f;

#include "BlockManager.h"
#include "Block.h"
#include "BlockType.h"
#include "Chunk.h"
#include "TextureManager.h"
Chunk *chunk;

// 输入处理
void processInput(const Uint8 *state)
{
    if (state[SDL_SCANCODE_W])
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (state[SDL_SCANCODE_S])
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (state[SDL_SCANCODE_A])
        camera.ProcessKeyboard(LEFTWARD, deltaTime);
    if (state[SDL_SCANCODE_D])
        camera.ProcessKeyboard(RIGHTWARD, deltaTime);
    if (state[SDL_SCANCODE_SPACE])
        camera.ProcessKeyboard(UP, deltaTime); // 空格向上
    if (state[SDL_SCANCODE_LSHIFT])
        camera.ProcessKeyboard(DOWN, deltaTime); // 左Shift向下
}

int main()
{
    // 初始化 SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "SDL 初始化失败: " << SDL_GetError() << std::endl;
        return -1;
    }

    // 设置 OpenGL 版本
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // 创建窗口
    SDL_Window *window = SDL_CreateWindow("OpenGL Cube",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          SCR_WIDTH, SCR_HEIGHT,
                                          SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    if (!window)
    {
        std::cerr << "窗口创建失败: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    SDL_GLContext context = SDL_GL_CreateContext(window);
    SDL_SetRelativeMouseMode(SDL_TRUE); // 捕获鼠标

    // 初始化 GLAD
    if (!gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    Shader shader("assets/shaders/basic.vert", "assets/shaders/basic.frag");

    // 初始化纹理共享
    TextureManager::init();
    // 创建一个区块
    chunk = new Chunk(glm::vec3(0.0f, 0.0f, 0.0f));
    chunk->generate();

    
    Block::initSharedMesh();
    int count = 0;

    // 渲染循环
    bool running = true;
    SDL_Event event;
    while (running)
    {
        float currentFrame = SDL_GetTicks() / 1000.0f;
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        const Uint8 *state = SDL_GetKeyboardState(NULL);
        processInput(state);

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                running = false;
            if (event.type == SDL_MOUSEMOTION)
            {
                float xoffset = event.motion.xrel;
                float yoffset = -event.motion.yrel; // 反转 y 轴
                camera.ProcessMouseMovement(xoffset, yoffset);
            }
        }

        // 清屏
        glClearColor(250.1f, 250.1f, 250.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 绘制立方体
        shader.use();

        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
                                                (float)SCR_WIDTH / (float)SCR_HEIGHT,
                                                0.1f, 100.0f);

        shader.setMat4("view", view);
        shader.setMat4("projection", projection);
        // 渲染区块
        count++;
        auto start = std::chrono::high_resolution_clock::now();
        chunk->render(shader);
        auto end = std::chrono::high_resolution_clock::now();
        float renderTime = std::chrono::duration<float, std::milli>(end - start).count();
        if(count >= 1000){std::cout << "Render Time: " << renderTime << " ms" << std::endl;count = 0;}

        SDL_GL_SwapWindow(window);
    }

    // 清理
    
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

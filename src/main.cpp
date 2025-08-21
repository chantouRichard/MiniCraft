#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <gl.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include "Shader.h"
#include "Camera3D.h"

#include <iostream>

// 屏幕大小
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// 相机
Camera3D camera;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// 时间
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// 输入处理
void processInput(const Uint8 *state)
{
    if (state[SDL_SCANCODE_W])
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (state[SDL_SCANCODE_S])
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (state[SDL_SCANCODE_A])
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (state[SDL_SCANCODE_D])
        camera.ProcessKeyboard(RIGHT, deltaTime);
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

    // 顶点数据
    float vertices[] = {
        -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f,
        -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,
        0.5f, 0.5f, 0.5f, 0.5f, 0.5f, -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f, 0.5f,
        -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, -0.5f,
        -0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, -0.5f};

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

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
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 绘制立方体
        shader.use();

        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
                                                (float)SCR_WIDTH / (float)SCR_HEIGHT,
                                                0.1f, 100.0f);

        shader.setMat4("model", model);
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        SDL_GL_SwapWindow(window);
    }

    // 清理
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

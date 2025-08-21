#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <gl.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include "Shader.h"
#include "Camera3D.h"

#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
unsigned int textureTop, textureSide, textureBottom;
int width, height, nrChannels;
unsigned char* data;
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

    float vertices[] = {
        // 顶部 (y = 0.5)
        -0.5f, 0.5f, -0.5f,  0.0f, 1.0f,
        0.5f, 0.5f, -0.5f,  1.0f, 1.0f,
        0.5f, 0.5f,  0.5f,  1.0f, 0.0f,
        0.5f, 0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, 0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, 0.5f, -0.5f,  0.0f, 1.0f,

        // 底部 (y = -0.5)
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 1.0f,
        0.5f, -0.5f,  0.5f, 1.0f, 0.0f,
        0.5f, -0.5f,  0.5f, 1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,

        // 前面 (z = 0.5)
        -0.5f, -0.5f, 0.5f,  0.0f, 1.0f,
        0.5f, -0.5f, 0.5f,  1.0f, 1.0f,
        0.5f,  0.5f, 0.5f,  1.0f, 0.0f,
        0.5f,  0.5f, 0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, 0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, 0.5f,  0.0f, 1.0f,

        // 后面 (z = -0.5)
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 1.0f,
        0.5f,  0.5f, -0.5f, 1.0f, 0.0f,
        0.5f,  0.5f, -0.5f, 1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,

        // 左面 (x = -0.5)
        -0.5f, -0.5f, -0.5f, 1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f, 1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, 0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, 0.0f, 0.0f,
        -0.5f, -0.5f,  0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 1.0f, 1.0f,

        // 右面 (x = 0.5)
        0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        0.5f,  0.5f, -0.5f, 0.0f, 0.0f,
        0.5f,  0.5f,  0.5f, 1.0f, 0.0f,
        0.5f,  0.5f,  0.5f, 1.0f, 0.0f,
        0.5f, -0.5f,  0.5f, 1.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
    };

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 位置
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // 纹理坐标
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // 纹理渲染
    // 顶部纹理
    glGenTextures(1, &textureTop);
    glBindTexture(GL_TEXTURE_2D, textureTop);
    // 设置纹理参数
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // 加载图片
    data = stbi_load("assets/textures/grass_top.png", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture: grass_top.png" << std::endl;
    }
    stbi_image_free(data);

    // 侧面纹理
    glGenTextures(1, &textureSide);
    glBindTexture(GL_TEXTURE_2D, textureSide);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    data = stbi_load("assets/textures/grass_side.png", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    stbi_image_free(data);

    // 底部纹理
    glGenTextures(1, &textureBottom);
    glBindTexture(GL_TEXTURE_2D, textureBottom);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    data = stbi_load("assets/textures/grass_bottom.png", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    stbi_image_free(data);


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
        // glDrawArrays(GL_TRIANGLES, 0, 36);
        // 绘制顶部
        glActiveTexture(GL_TEXTURE0);       // 激活纹理单元 0
        glBindTexture(GL_TEXTURE_2D, textureTop); // 绑定顶部纹理
        shader.setInt("texture1", 0);       // 告诉 shader 使用纹理单元 0
        glDrawArrays(GL_TRIANGLES, 0, 6);   // 绘制顶部六个顶点

        // 绘制底部
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureBottom);
        shader.setInt("texture1", 0);
        glDrawArrays(GL_TRIANGLES, 6, 6);   // 底部六个顶点

        // 绘制四个侧面
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureSide);
        shader.setInt("texture1", 0);
        for (int i = 12; i < 36; i += 6)
        {
            glDrawArrays(GL_TRIANGLES, i, 6);
        }

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

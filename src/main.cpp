#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <gl.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <iostream>
#include <chrono>

#include "Shader.h"
#include "Camera3D.h"
#include "BlockManager.h"
#include "Block.h"
#include "BlockType.h"
#include "Chunk.h"
#include "TextureManager.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// 全局常量
const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 800;

// 全局对象
Camera3D camera;
Chunk* chunk = nullptr;

// 时间控制
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// 区块分批渲染
#include <queue>
#include <vector>
#include <memory>
#include <algorithm>

struct ChunkTask {
    std::shared_ptr<Chunk> chunk;
    bool generated = false;
};

std::queue<ChunkTask> pendingChunks;
std::vector<std::shared_ptr<Chunk>> readyChunks;

void initChunks(int playerChunkX, int playerChunkZ) {
    int radius = 2;               // 半径
    float chunkSpacing = CHUNK_SIZE;

    struct ChunkWithDist {
        std::shared_ptr<Chunk> chunk;
        int dist; // 曼哈顿距离
    };
    std::vector<ChunkWithDist> chunks;

    // 扫描半径范围内的 Chunk
    for(int x = playerChunkX - radius; x <= playerChunkX + radius; ++x){
        for(int z = playerChunkZ - radius; z <= playerChunkZ + radius; ++z){
            // 计算欧几里得距离，保证是圆形区域
            int dx = x - playerChunkX;
            int dz = z - playerChunkZ;
            if(dx*dx + dz*dz <= radius*radius){  // 在半径内
                int dist = abs(dx) + abs(dz);   // 曼哈顿距离排序用
                auto chunk = std::make_shared<Chunk>(glm::vec3(x*chunkSpacing, 0, z*chunkSpacing));
                chunks.push_back({chunk, dist});
            }
        }
    }

    // 按曼哈顿距离排序，距离近的先生成
    std::sort(chunks.begin(), chunks.end(), [](const ChunkWithDist &a, const ChunkWithDist &b){
        return a.dist < b.dist;
    });

    // 入队
    for(auto &c : chunks){
        pendingChunks.push({c.chunk, false});
    }
}


void updateChunkGeneration(int chunksPerFrame = 1) {
    int count = 0;
    while(!pendingChunks.empty() && count < chunksPerFrame){
        auto &task = pendingChunks.front();
        if(!task.generated){
            task.chunk->generate();  // 耗时操作
            task.generated = true;
            readyChunks.push_back(task.chunk);
            count++;
        }
        pendingChunks.pop();
    }
}


// -------------------- 输入处理 --------------------
void processInput(const Uint8* state)
{
    if (state[SDL_SCANCODE_W]) camera.ProcessKeyboard(FORWARD, deltaTime);
    if (state[SDL_SCANCODE_S]) camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (state[SDL_SCANCODE_A]) camera.ProcessKeyboard(LEFTWARD, deltaTime);
    if (state[SDL_SCANCODE_D]) camera.ProcessKeyboard(RIGHTWARD, deltaTime);
    if (state[SDL_SCANCODE_SPACE]) camera.ProcessKeyboard(UP, deltaTime);
    if (state[SDL_SCANCODE_LSHIFT]) camera.ProcessKeyboard(DOWN, deltaTime);
}

// 鼠标处理
void handleMouseMotion(const SDL_Event& event)
{
    float xoffset = event.motion.xrel;
    float yoffset = -event.motion.yrel; // 反转 y 轴
    camera.ProcessMouseMovement(xoffset, yoffset);
}

// -------------------- 初始化函数 --------------------
bool initSDL(SDL_Window** window, SDL_GLContext* context)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "SDL 初始化失败: " << SDL_GetError() << std::endl;
        return false;
    }

    // OpenGL 版本 3.3 Core
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    *window = SDL_CreateWindow("OpenGL Cube",
                               SDL_WINDOWPOS_CENTERED,
                               SDL_WINDOWPOS_CENTERED,
                               SCR_WIDTH, SCR_HEIGHT,
                               SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!*window)
    {
        std::cerr << "窗口创建失败: " << SDL_GetError() << std::endl;
        return false;
    }

    *context = SDL_GL_CreateContext(*window);
    SDL_SetRelativeMouseMode(SDL_TRUE); // 捕获鼠标

    if (!gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    glEnable(GL_DEPTH_TEST);
    return true;
}

Shader initResources()
{
    // 加载 Shader
    Shader shader("assets/shaders/basic.vert", "assets/shaders/basic.frag");

    // 初始化纹理管理器
    TextureManager::init();

    // 创建区块
    initChunks(camera.Position.x / CHUNK_SIZE, camera.Position.z / CHUNK_SIZE);

    // 初始化方块共享网格
    Block::initSharedMesh();

    return shader;
}

// -------------------- 渲染函数 --------------------
void renderScene(Shader& shader)
{
    // 清屏
    glClearColor(0.98f, 0.98f, 0.98f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shader.use();

    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
                                            (float)SCR_WIDTH / (float)SCR_HEIGHT,
                                            0.1f, 100.0f);

    shader.setMat4("view", view);
    shader.setMat4("projection", projection);

    // 渲染区块
    static int frameCount = 0;
    auto start = std::chrono::high_resolution_clock::now();
    for(auto &chunk : readyChunks){
        chunk->render(shader);
    }
    auto end = std::chrono::high_resolution_clock::now();

    float renderTime = std::chrono::duration<float, std::milli>(end - start).count();
    if (++frameCount >= 1000)
    {
        std::cout << "Render Time: " << renderTime << " ms" << std::endl;
        frameCount = 0;
    }
}

// -------------------- 主循环 --------------------
void gameLoop(SDL_Window* window, Shader& shader)
{
    bool running = true;
    SDL_Event event;

    while (running)
    {
        float currentFrame = SDL_GetTicks() / 1000.0f;
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        const Uint8* state = SDL_GetKeyboardState(NULL);
        processInput(state);

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                running = false;
            if (event.type == SDL_MOUSEMOTION)
                handleMouseMotion(event);
        }
        updateChunkGeneration(1); // 每帧生成 1 个 Chunk

        renderScene(shader);
        SDL_GL_SwapWindow(window);
    }
}

// -------------------- 清理 --------------------
void cleanup(SDL_Window* window, SDL_GLContext context)
{
    delete chunk;
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

// -------------------- 主函数 --------------------
int main()
{
    SDL_Window* window = nullptr;
    SDL_GLContext context;

    if (!initSDL(&window, &context))
        return -1;

    Shader shader = initResources();
    gameLoop(window, shader);
    cleanup(window, context);

    return 0;
}

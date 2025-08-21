cmake_minimum_required(VERSION 3.10)
project(MiniCraft)

set(CMAKE_CXX_STANDARD 17)

# 包含头文件
include_directories(
    include
    lib/SDL2/include
    lib/glew/include
    lib/glm
    external/stb
    external/glad
)

find_package(OpenGL REQUIRED)

# 源文件
file(GLOB SOURCES
    ${CMAKE_SOURCE_DIR}/src/*.cpp
    ${CMAKE_SOURCE_DIR}/external/glad/gl.c
)

# Windows / MinGW: OpenGL 库名是 opengl32
if (WIN32)
    set(OPENGL_LIB opengl32)
else()
    find_package(OpenGL REQUIRED)
    set(OPENGL_LIB ${OPENGL_LIBRARIES})
endif()

# 生成可执行文件
add_executable(MiniCraft ${SOURCES} external/glad/gl.c)

# 链接 SDL2main 和 SDL2（手动指定路径）
target_link_libraries(MiniCraft
    "${CMAKE_SOURCE_DIR}/lib/SDL2/lib/libSDL2main.a"
    "${CMAKE_SOURCE_DIR}/lib/SDL2/lib/libSDL2.a"
    ${OPENGL_LIB}
    ${OPENGL_LIBRARIES}
    "${CMAKE_SOURCE_DIR}/lib/glew/lib/libglew32.a"
    setupapi
    imm32
    ole32
    uuid
    version
    # -mwindows   # 如果你不想弹出控制台窗口
    ${SDL2_LIBRARIES}
    mingw32      # MinGW 初始化库
    SDL2     # 如果你还在用 libSDL2main.a
    winmm        # 多媒体音频计时库
)

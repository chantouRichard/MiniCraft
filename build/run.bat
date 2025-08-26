@echo off
REM 进入 build 目录
cd /d "%~dp0build"

REM 使用 CMake 构建项目
cmake --build .

REM 构建完成后运行 MiniCraft.exe
if exist MiniCraft.exe (
    echo 运行 MiniCraft.exe...
    MiniCraft.exe
) else (
    echo 错误：MiniCraft.exe 未找到！
)

pause

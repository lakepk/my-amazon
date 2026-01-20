# 1. 指定 CMake 的最低版本要求
# FetchContent 功能需要至少 3.11 版本
cmake_minimum_required(VERSION 3.11)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# 2. 定义项目名称和版本
project(MyBoardGame VERSION 1.0)

if(MINGW)
    # 强制静态链接标准库和线程库
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")
    
    # 如果你使用了外部库（如 raylib），也确保它们是静态链接的
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
endif()

# 3. 设置 C++ 标准
# 我们使用 C++17，这对学习者很友好，支持很多现代特性
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -----------------------------------------------------------------------------
# 4. 引入 Raylib (这是最关键的一步)
# -----------------------------------------------------------------------------
# 告诉 CMake 我们要用 FetchContent 模块
include(FetchContent)

# 声明我们要从 GitHub 下载 Raylib
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG master  # 这里指定版本，master 是最新版，也可以写具体版本号如 "4.5.0"
)

# 开始下载并使 Raylib 可用 (这一步在第一次运行时会花一点时间下载)
FetchContent_MakeAvailable(raylib)

# -----------------------------------------------------------------------------
# 5. 定义你的可执行文件
# -----------------------------------------------------------------------------
# 告诉 CMake，我们的游戏叫 "MyGame"，源文件在 src/main.cpp

add_executable(MyGame 
    src/main.cpp 
    src/Board.cpp 
    src/MCTS.cpp
    src/GameManager.cpp
)

# -----------------------------------------------------------------------------
# 6. 链接库
# -----------------------------------------------------------------------------
# 把 Raylib 的功能“连接”到你的游戏上
# PRIVATE 意味着 Raylib 只是你的游戏内部使用
target_link_libraries(MyGame PRIVATE raylib)



# 编译Board.cpp文件

# 如果你是 Windows 用户，为了不让控制台窗口总是弹出来（发布时用），可以解开下面这行的注释：
set_target_properties(MyGame PROPERTIES WIN32_EXECUTABLE ON)
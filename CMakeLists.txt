cmake_minimum_required(VERSION 3.10)
project(TinyWebServer)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加头文件搜索路径
include_directories(${PROJECT_SOURCE_DIR}/include)

add_executable(tiny_web_server src/main.cpp)



# 测试
add_executable(test_log test/test_log.cpp src/log.cpp)
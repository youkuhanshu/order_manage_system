cmake_minimum_required(VERSION 3.5) # CMake install : https://cmake.org/download/
project(order_system LANGUAGES CXX)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_PREFIX_PATH "D:/Qt/6.11.1/mingw_64") # Qt Kit Dir
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Qt6 COMPONENTS Widgets Network REQUIRED) # Qt COMPONENTS

# 添加项目根目录的 include 路径
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)

# 用 GLOB + CONFIGURE_DEPENDS：新增/删除 src 下的 .cpp 时会自动重新配置，
# 避免出现“新加的文件没被编译 → 链接报 undefined reference”的问题。
file(GLOB srcs CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

# 添加 storage 目录下的源文件（不在 src 目录内，需显式加入）
list(APPEND srcs ${CMAKE_CURRENT_SOURCE_DIR}/../storage/FileManager.cpp)
# 添加orderservice目录下的源文件
list(APPEND srcs ${CMAKE_CURRENT_SOURCE_DIR}/../order/src/Order_mgr.cpp)
# 添加排队服务的源文件
list(APPEND srcs ${CMAKE_CURRENT_SOURCE_DIR}/../queue/src/queue_service.cpp)
# 添加评论服务的源文件
list(APPEND srcs ${CMAKE_CURRENT_SOURCE_DIR}/../comment/src/comment_service.cpp)

# Specify MSVC UTF-8 encoding   
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

add_executable(${PROJECT_NAME}
    WIN32 # If you need a terminal for debug, please comment this statement 
    ${srcs} 
) 
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets Qt6::Network) # Qt5 Shared Library
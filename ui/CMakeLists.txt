cmake_minimum_required(VERSION 3.5) # CMake install : https://cmake.org/download/
project(order_system LANGUAGES CXX)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_PREFIX_PATH "D:/Qt/6.11.1/mingw_64") # Qt Kit Dir
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Qt6 COMPONENTS Widgets REQUIRED) # Qt COMPONENTS

# 添加项目根目录的 include 路径
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)

aux_source_directory(./src srcs)

# 添加 storage 目录下的源文件
list(APPEND srcs ${CMAKE_CURRENT_SOURCE_DIR}/../storage/FileManager.cpp)

# Specify MSVC UTF-8 encoding   
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

add_executable(${PROJECT_NAME}
    WIN32 # If you need a terminal for debug, please comment this statement 
    ${srcs} 
) 
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets) # Qt5 Shared Library
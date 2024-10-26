cmake_minimum_required(VERSION 3.0)
project(tinyrenderer)

set(CMAKE_CXX_STANDARD 17)
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/3rdparty)

find_library(GLFW3_LIBRARY glfw)

add_library(glad
    3rdparty/glad.c
)
set(GLAD_LIBRARY glad)

add_executable(main
    src/main.cpp
    src/application.cpp
    src/model.cpp
    src/renderer.cpp
    src/utils.cpp
)

target_link_libraries(main ${GLFW3_LIBRARY} ${GLAD_LIBRARY})
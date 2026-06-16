cmake_minimum_required(VERSION 3.20)
project(tinyrenderer)

set(CMAKE_CXX_STANDARD 17)
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/3rdparty)

find_library(GLFW3_LIBRARY glfw)

add_library(glad
    3rdparty/glad/glad.c
)
set(GLAD_LIBRARY glad)

add_library(obj_loader
    3rdparty/obj_loader/tiny_obj_loader.cpp
)
set(OBJ_LOADER_LIBRARY obj_loader)

add_library(stbi
    3rdparty/stb_image/stb_image.c
)
set(STBI_LIBRARY stbi)

file(GLOB SRC_FILES CONFIGURE_DEPENDS
    "src/*.cpp"
)
add_executable(main ${SRC_FILES})

target_link_libraries(main ${GLFW3_LIBRARY} ${GLAD_LIBRARY} ${OBJ_LOADER_LIBRARY} ${STBI_LIBRARY})
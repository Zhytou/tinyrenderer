cmake_minimum_required(VERSION 3.10)
project(tinyglrenderer)

set(CMAKE_CXX_STANDARD 20)
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

add_library(imgui
    3rdparty/imgui/imgui.cpp
    3rdparty/imgui/imgui_draw.cpp
    3rdparty/imgui/imgui_widgets.cpp
    3rdparty/imgui/imgui_tables.cpp
    3rdparty/imgui/imgui_impl_glfw.cpp
    3rdparty/imgui/imgui_impl_opengl3.cpp
)
set(IMGUI_LIBRARY imgui)

file(GLOB SRC_FILES CONFIGURE_DEPENDS
    "src/*.cpp"
)
add_executable(app ${SRC_FILES})

target_link_libraries(app ${GLFW3_LIBRARY} ${GLAD_LIBRARY} ${OBJ_LOADER_LIBRARY} ${STBI_LIBRARY} ${IMGUI_LIBRARY})
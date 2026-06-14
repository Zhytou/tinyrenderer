#pragma once

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <memory>

#include "renderer.hpp"
#include "scene.hpp"

namespace tinyrenderer {

class Application {
   public:
    Application(uint32_t width, uint32_t height, const std::string& title);
    ~Application();

    // initialize application window and OpenGL context
    void setup(const std::string& title);
    // destroy OpenGL resources and terminate GLFW
    void shutdown();

    // load scene(model/material/light/camera)
    void load(const std::string& scenePath);

    // run application main loop
    void run();

   private:
    GLFWwindow* m_window;
    Renderer m_renderer;
    Scene m_scene;
    uint32_t m_width, m_height;
};

}  // namespace tinyrenderer

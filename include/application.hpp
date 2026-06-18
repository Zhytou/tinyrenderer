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
    void processInput(float deltaTime);
    void drawHUD(float deltaTime);
    float calculateFPS(float deltaTime);

    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
    static void frameSizeCallback(GLFWwindow* window, int width, int height);

    GLFWwindow* m_window;
    Renderer m_renderer;
    Scene m_scene;
    uint32_t m_width, m_height;

    bool m_isMouseDragging = false;
    double m_lastX, m_lastY;
};

}  // namespace tinyrenderer

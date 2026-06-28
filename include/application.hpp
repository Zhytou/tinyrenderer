#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <memory>
#include <string>
#include <unordered_map>

#include "editor.hpp"
#include "renderer.hpp"
#include "scene.hpp"

namespace tinyglrenderer {

class Application {
   public:
    Application(int width, int height, const std::string& title);
    ~Application();

    // initialize application window and OpenGL context
    void setup(int width, int height, const std::string& title);
    // destroy OpenGL resources and terminate GLFW
    void shutdown();

    // load scene(model/material/light/camera)
    void load(const std::string& scenePath);
    // run application main loop
    void run();

    // resize application window
    void resize(int width, int height);

   private:
    void processInput(float deltaTime);
    DisplayInfo getDisplayInfo(float deltaTime);
    float calculateFPS(float deltaTime);

    static void scrollCallback(GLFWwindow* window, double scrollX, double scrollY);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void cursorPosCallback(GLFWwindow* window, double cursorX, double cursorY);
    static void windowSizeCallback(GLFWwindow* window, int width, int height);
    static void windowPosCallback(GLFWwindow* window, int x, int y);

    GLFWwindow* m_window;
    Renderer m_renderer;
    RendererSetting m_rendererSetting;
    Editor m_editor;
    EditorSetting m_editorSetting;
    Scene m_scene;

    bool m_mouseLeftDragging  = false;
    bool m_mouseRightDragging = false;
    bool m_maximized          = false; // indicator of fullscreen window

    // Since 'glfwGetWindowPos' and 'glfwGetWindowSize' both return
    // cached values, store the window position and size when dragging begins.
    std::pair<double, double> m_rotatedCursorLocalPos  = {0.0, 0.0}; // local cursor position in window coordinates for camera rotation
    std::pair<double, double> m_draggedCursorGlobalPos = {0.0, 0.0}; // global cursor position in screen coordinates for titlebar dragging
    std::pair<int, int> m_draggedWinPos                = {0, 0};

    // Due to a known issue in Win32/GLFW where 'glfwMaximizeWindow' fails to
    // position and scale an UNDECORATED window properly (causing offset artifacts
    // like X/Y shifted from origin and revealing background apps), we bypass the
    // OS maximization entirely. We manually clamp the window bounds to these
    // absolute physical display dimensions to force a perfect seamless layout.
    std::pair<int, int> m_restoredWinPos   = {0, 0};       // global window position in screen coordinates for window restoration
    std::pair<int, int> m_restoredWinSize  = {1280, 720};  // 1720p by default
    std::pair<int, int> m_maximizedWinSize = {2560, 1440}; // 2k by default
};

} // namespace tinyglrenderer

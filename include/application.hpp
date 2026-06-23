#pragma once

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <memory>

#include "renderer.hpp"
#include "scene.hpp"

namespace tinyglrenderer {

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
    float calculateFPS(float deltaTime);
    void applyTheme();
    void drawUI(float deltaTime);
    void drawSideBar(float deltaTime);
    void drawHUD(float deltaTime);

    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
    static void frameSizeCallback(GLFWwindow* window, int width, int height);

    GLFWwindow* m_window;
    Renderer m_renderer;
    Scene m_scene;
    uint32_t m_width, m_height;

    bool m_mouseRightDragging = false;
    bool m_mouseLeftDragging  = false;

    enum class SidebarTab {
        None,      // 全收起状态
        Renderer,  // 渲染器设置
        Lights,    // 光源设置
        Models     // 模型设置
    };

    SidebarTab m_currentTab = SidebarTab::Renderer;  // 默认选中渲染器

    float m_activityBarWidth = 45.0f;   // 左侧图标条的宽度
    float m_sideBarWidth     = 280.0f;  // 调参面板的宽度
    double m_lastX, m_lastY;
};

}  // namespace tinyglrenderer

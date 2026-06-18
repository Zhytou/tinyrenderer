
#include "application.hpp"

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <rapidjson/document.h>

#include <filesystem>
#include <fstream>
#include <glm/gtc/matrix_transform.hpp>
#include <stdexcept>

#include "staticresource.hpp"
#include "utils.hpp"

namespace tinyrenderer {
Application::Application(uint32_t width, uint32_t height, const std::string& title) : m_width(width), m_height(height), m_renderer(width, height) {
    // initialize glfw and glad
    std::cout << "Initializing GLFW & GLAD for [" << title << "]\n";
    setup(title);

    // initialize renderer
    std::cout << "Running OpenGL Renderer [" << glGetString(GL_RENDERER) << "]\n";
    m_renderer.setup();

    // initialize static resources
    StaticResource::getInstance().initialize();
}

Application::~Application() {
    // destroy static resources
    StaticResource::getInstance().destroy();

    // clear renderer resources
    m_renderer.shutdown();

    // clear scene resources
    m_scene.destroy();

    // clear glfw resources
    shutdown();
}

void Application::setup(const std::string& title) {
    // glfw initialization
    if (!glfwInit()) {
        throw std::runtime_error("Application::setup: Failed to initialize GLFW library");
    }
    // set opengl version
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    // set z-buffer bits, otherwise glenbale(GL_DEPTH_TEST) will not work
    glfwWindowHint(GLFW_DEPTH_BITS, 24);

    // glfw window creation
    m_window = glfwCreateWindow(m_width, m_height, title.c_str(), nullptr, nullptr);
    if (!m_window) {
        throw std::runtime_error("Application::setup: Failed to create OpenGL context");
    }

    // glfw context settings(make sure it is all set before glad initialization)
    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1);

    // glfw callbacks
    // set window pointer for later use in callback function
    glfwSetWindowUserPointer(m_window, this);
    // set callback function
    glfwSetScrollCallback(m_window, Application::scrollCallback);
    glfwSetMouseButtonCallback(m_window, Application::mouseButtonCallback);
    glfwSetFramebufferSizeCallback(m_window, Application::frameSizeCallback);
    glfwSetCursorPosCallback(m_window, Application::cursorPosCallback);

    // glad initialization(load all OpenGL function pointers)
    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
        throw std::runtime_error("Application::setup: Failed to initialize OpenGL extensions loader");
    }

    // imgui initialization
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init("#version 410");
}

void Application::shutdown() {
    // destroy imgui context
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // destroy glfw window
    if (m_window) {
        glfwDestroyWindow(m_window);
    }
    glfwTerminate();
}

void Application::load(const std::string& scenePath) {
    std::ifstream file{scenePath};
    if (!file.is_open()) {
        throw std::runtime_error("Application::load: Could not open file: " + scenePath);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();

    m_scene.initialize(buffer.str());
    m_renderer.prepare(m_scene);
}

void Application::run() {
    float lastFrame = static_cast<float>(glfwGetTime());  // in seconds
    while (!glfwWindowShouldClose(m_window)) {
        float currFrame = static_cast<float>(glfwGetTime());  // in seconds
        float deltaTime = currFrame - lastFrame;
        lastFrame       = currFrame;

        // Process input
        processInput(deltaTime);

        // Update and render scene
        m_renderer.update(m_scene);
        m_renderer.render(m_scene);

        // Draw head-up display
        drawHUD(deltaTime);

        // Swap buffers and poll events
        glfwSwapBuffers(m_window);
        glfwPollEvents();
    }
}

void Application::processInput(float deltaTime) {
    auto& camera = m_scene.getCamera();

    if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(m_window, GLFW_KEY_UP) == GLFW_PRESS) {
        camera->move(CameraMovement::UPWARD, deltaTime);
    }
    if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(m_window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        camera->move(CameraMovement::DOWNWARD, deltaTime);
    }
    if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(m_window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        camera->move(CameraMovement::LEFT, deltaTime);
    }
    if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(m_window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        camera->move(CameraMovement::RIGHT, deltaTime);
    }

    if (glfwGetKey(m_window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        camera->reset();
    }
    if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(m_window, true);
    }
}

void Application::drawHUD(float deltaTime) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    {
        const float padding   = 10.0f;
        ImVec2 windowPos      = ImVec2(static_cast<float>(m_width) - padding, padding);
        ImVec2 windowPosPivot = ImVec2(1.0f, 0.0f);  // right top corner (1.0, 0.0)

        ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always, windowPosPivot);
        ImGui::SetNextWindowBgAlpha(0.35f);  // slightly transparent black background, not blocking 3D scene

        // Make the window invisible and not interactable with the user
        ImGuiWindowFlags windowFlags =
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoFocusOnAppearing |
            ImGuiWindowFlags_NoNav |
            ImGuiWindowFlags_NoMove;

        static uint32_t prevDrawCall = 0;
        uint32_t currDrawCall        = m_renderer.getDrawCall();

        if (ImGui::Begin("HUD", nullptr, windowFlags)) {
            ImGui::Text("PERFORMANCE");
            ImGui::Separator();
            ImGui::Text("FPS       : %.1f", calculateFPS(deltaTime));
            ImGui::Text("Frame Time: %.2f ms", deltaTime * 1000.0f);
            ImGui::Text("Draw Call: %d draw calls", currDrawCall - prevDrawCall);

            ImGui::Spacing();
            ImGui::Text("STATE");
            ImGui::Separator();
            const auto& camera = m_scene.getCamera();
            glm::vec3 pos      = camera->getEye();
            glm::vec3 target   = camera->getTarget();
            glm::vec3 front    = glm::normalize(target - pos);
            ImGui::Text("Camera Position : (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);
            ImGui::Text("Camera FrontVec : (%.2f, %.2f, %.2f)", front.x, front.y, front.z);
            ImGui::Text("Shadow Mapping : %s", m_renderer.isShadowMapEnabled() ? "On" : "Off");
            ImGui::Text("Environment IBL : %s", m_renderer.isIBLEnabled() ? "On" : "Off");
        }
        ImGui::End();

        prevDrawCall = currDrawCall;
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

float Application::calculateFPS(float deltaTime) {
    static float timer    = 0.0f;
    static int frameCount = 0;
    static float fps      = 0.0f;

    timer += deltaTime;
    frameCount++;

    if (timer >= 0.5f) {
        float nfps = static_cast<float>(frameCount) / timer;

        if (fps == 0.0f) {
            fps = nfps;
        } else {
            fps = (fps * 0.95f + nfps * 0.05f);
        }

        timer      = 0.0f;
        frameCount = 0;
    }

    return fps;
}

void Application::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    if (!app) return;

    auto& camera = app->m_scene.getCamera();
    if (camera) {
        camera->zoom(static_cast<float>(yoffset));
    }
}

void Application::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    if (!app) return;

    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            app->m_isMouseDragging = true;

            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            app->m_lastX = static_cast<float>(xpos);
            app->m_lastY = static_cast<float>(ypos);
        } else if (action == GLFW_RELEASE) {
            app->m_isMouseDragging = false;
        }
    }
}

void Application::cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    if (!app) return;
    if (!app->m_isMouseDragging) return;

    float currX   = static_cast<float>(xpos);
    float currY   = static_cast<float>(ypos);
    float offsetX = currX - app->m_lastX;
    float offsetY = app->m_lastY - currY;
    app->m_lastX  = currX;
    app->m_lastY  = currY;

    auto& camera = app->m_scene.getCamera();
    if (camera) {
        camera->rotate(offsetX, offsetY);
    }
}

void Application::frameSizeCallback(GLFWwindow* window, int width, int height) {
    auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    if (!app) return;
    // TODO: resize renderer
}

}  // namespace tinyrenderer

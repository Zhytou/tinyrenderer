
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

namespace tinyglrenderer {
Application::Application(uint32_t width, uint32_t height, const std::string& title) : m_width(width), m_height(height) {
    if (width < 800 || height < 600) {
        throw std::runtime_error("Application::Application: Window size must be at least 800x600");
    }

    // initialize glfw and glad
    std::cout << "Initializing GLFW & GLAD for [" << title << "]\n";
    setup(title);

    // initialize renderer
    std::cout << "Running OpenGL Renderer [" << glGetString(GL_RENDERER) << "]\n";
    m_renderer.setup();

    // initialize editor
    m_editor.setup();

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
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
    glfwWindowHint(GLFW_DEPTH_BITS, 24); // allocate at least 24 depth bits for glfw swapbuffers, otherwise glenbale(GL_DEPTH_TEST) may not work
    glfwWindowHint(GLFW_STENCIL_BITS, 8);

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
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(
        [](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
            if (severity == GL_DEBUG_SEVERITY_HIGH || severity == GL_DEBUG_SEVERITY_MEDIUM) {
                std::cerr << "[OpenGL Error] " << message << std::endl;
            }
        },
        nullptr);

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
    static float lastFrame       = static_cast<float>(glfwGetTime()); // in seconds
    static RenderSetting setting = m_renderer.getSetting();
    static DisplayInfo info      = getDisplayInfo(0);

    while (!glfwWindowShouldClose(m_window)) {
        float currFrame = static_cast<float>(glfwGetTime()); // in seconds
        float deltaTime = currFrame - lastFrame;
        lastFrame       = currFrame;

        // Process input
        processInput(deltaTime);

        // Update and render scene
        m_renderer.update(m_scene);
        m_renderer.render(m_scene);

        // Draw user interface
        m_editor.draw(m_scene, setting, info);

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

DisplayInfo Application::getDisplayInfo(float deltaTime) {
    return DisplayInfo{
        .fps       = calculateFPS(deltaTime),
        .deltaTime = deltaTime,
        .drawCall  = m_renderer.getDrawCall(),
    };
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
    if (!app)
        return;

    auto& camera = app->m_scene.getCamera();
    if (camera) {
        camera->zoom(static_cast<float>(yoffset));
    }
}

void Application::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    if (!app)
        return;

    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            app->m_mouseRightDragging = true;

            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            app->m_lastX = static_cast<float>(xpos);
            app->m_lastY = static_cast<float>(ypos);
        } else if (action == GLFW_RELEASE) {
            app->m_mouseRightDragging = false;
        }
    } else if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            app->m_mouseLeftDragging = true;

            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            app->m_lastX = static_cast<float>(xpos);
            app->m_lastY = static_cast<float>(ypos);
        } else if (action == GLFW_RELEASE) {
            app->m_mouseLeftDragging = false;
        }
    }
}

void Application::cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    if (!app)
        return;
    if (!app->m_mouseRightDragging && !app->m_mouseLeftDragging)
        return;

    float currX   = static_cast<float>(xpos);
    float currY   = static_cast<float>(ypos);
    float lastX   = app->m_lastX;
    float lastY   = app->m_lastY;
    float offsetX = currX - lastX;
    float offsetY = lastY - currY;

    auto& camera = app->m_scene.getCamera();
    if (camera) {
        if (app->m_mouseRightDragging) {
            camera->rotate(offsetX, offsetY);
        } else if (app->m_mouseLeftDragging) {
            // camera->move(offsetX > 0 ? CameraMovement::RIGHT : CameraMovement::LEFT, offsetX / camera->getSpeed());
            // camera->move(offsetY > 0 ? CameraMovement::UPWARD : CameraMovement::DOWNWARD, offsetY / camera->getSpeed());
        }
    }

    app->m_lastX = currX;
    app->m_lastY = currY;
}

void Application::frameSizeCallback(GLFWwindow* window, int width, int height) {
    auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    if (!app)
        return;

    app->m_width  = width;
    app->m_height = height;
    app->m_renderer.resize(width, height);
}

} // namespace tinyglrenderer

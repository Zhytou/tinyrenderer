
#include "application.hpp"

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
        throw std::runtime_error("Failed to initialize GLFW library");
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
        throw std::runtime_error("Failed to create OpenGL context");
    }

    // glfw context settings(make sure it is all set before glad initialization)
    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1);

    // glfw callbacks
    // set window pointer for later use in callback function
    glfwSetWindowUserPointer(m_window, this);
    // set callback function
    glfwSetWindowSizeCallback(m_window, [](GLFWwindow* window, int w, int h) {
        glViewport(0, 0, w, h);
    });

    // glad initialization(load all OpenGL function pointers)
    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
        throw std::runtime_error("Failed to initialize OpenGL extensions loader");
    }
}

void Application::shutdown() {
    if (m_window) {
        glfwDestroyWindow(m_window);
    }
    glfwTerminate();
}

void Application::load(const std::string& scenePath) {
    std::ifstream file{scenePath};
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + scenePath);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();

    m_scene.initialize(buffer.str());
}

void Application::run() {
    m_renderer.prepare(m_scene);
    while (!glfwWindowShouldClose(m_window)) {
        m_renderer.render(m_scene);
        glfwSwapBuffers(m_window);
        glfwPollEvents();
    }
}

}  // namespace tinyrenderer

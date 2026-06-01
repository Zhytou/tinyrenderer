
#include "application.hpp"

#include <rapidjson/document.h>

#include <filesystem>
#include <glm/gtc/matrix_transform.hpp>
#include <stdexcept>

#include "utils.hpp"

namespace tinyrenderer {
Application::Application(uint32_t width, uint32_t height, const std::string& title) : m_width(width), m_height(height), m_renderer(width, height) {
    // initialize glfw and glad
    setup(title);
    // initialize renderer
    m_renderer.setup();

    std::cout << "Running OpenGL Renderer [" << glGetString(GL_RENDERER) << "]\n";
}

Application::~Application() {
    // clear renderer resources
    m_renderer.shutdown();

    // clear scene resources
    m_scene.lights.clear();
    m_scene.models.clear();

    // clear glfw resources
    shutdown();
}

void Application::shutdown() {
    if (m_window) {
        glfwDestroyWindow(m_window);
    }
    glfwTerminate();
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
    glfwSwapInterval(-1);

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

void Application::load(const std::string& config) {
    std::string json = readText(config);
    rapidjson::Document doc;
    if (doc.Parse(json.c_str()).HasParseError()) {
        throw std::runtime_error("Error parsing JSON");
    }
    auto getVec3 = [](rapidjson::Value& arr) -> glm::vec3 {
        if (!arr.IsArray() || arr.Size() != 3 || !arr[0].IsNumber() || !arr[1].IsNumber() || !arr[2].IsNumber()) {
            throw std::runtime_error("Invalid array size or element type");
        }

        return glm::vec3{arr[0].GetFloat(), arr[1].GetFloat(), arr[2].GetFloat()};
    };

    // TODO: add point light support
    // lights
    if (doc.HasMember("lights")) {
        for (int i = 0; i < doc["lights"]["directionallight"].Size(); i++) {
            m_scene.lights.emplace_back(std::make_shared<DirectionalLight>(getVec3(doc["lights"]["directionallight"][i]["direction"]), getVec3(doc["lights"]["directionallight"][i]["color"])));
        }
    }

    // models
    if (doc.HasMember("models")) {
        for (int i = 0; i < doc["models"].Size(); i++) {
            std::string baseDir   = doc["models"][i]["baseDir"].GetString();
            std::string modelName = doc["models"][i]["name"].GetString();
            glm::vec3 translate   = doc["models"][i]["transform"].HasMember("translate") ? getVec3(doc["models"][i]["transform"]["translate"]) : glm::vec3(0.0f);
            glm::vec3 rotate      = doc["models"][i]["transform"].HasMember("rotate") ? getVec3(doc["models"][i]["transform"]["rotate"]) : glm::vec3(0.0f);
            glm::vec3 scale       = doc["models"][i]["transform"].HasMember("scale") ? getVec3(doc["models"][i]["transform"]["scale"]) : glm::vec3(1.0f);

            glm::mat4 transform = glm::mat4(1.0f);
            transform           = glm::translate(transform, translate);
            transform           = glm::rotate(transform, glm::radians(rotate.x), glm::vec3(1.0f, 0.0f, 0.0f));
            transform           = glm::rotate(transform, glm::radians(rotate.y), glm::vec3(0.0f, 1.0f, 0.0f));
            transform           = glm::rotate(transform, glm::radians(rotate.z), glm::vec3(0.0f, 0.0f, 1.0f));
            transform           = glm::scale(transform, scale);
            m_scene.models.emplace_back(std::make_shared<Model>(baseDir, modelName, transform));
            auto [xyzi1, xyzi2] = m_scene.models.back()->getBoundingBox();
            std::cout << "No" << i << ": " << xyzi1 << ' ' << xyzi2 << '\n';
            m_scene.xyz1 = glm::min(m_scene.xyz1, xyzi1);
            m_scene.xyz2 = glm::max(m_scene.xyz2, xyzi2);
        }
    }

    // camera
    if (doc.HasMember("camera")) {
        m_scene.camera.setEye(getVec3(doc["camera"]["eye"]));
        m_scene.camera.setTarget(getVec3(doc["camera"]["target"]));
        m_scene.camera.setUp(getVec3(doc["camera"]["up"]));
        m_scene.camera.setFov(doc["camera"]["fov"].GetFloat());
        m_scene.camera.setNear(doc["camera"]["near"].GetFloat());
        m_scene.camera.setFar(doc["camera"]["far"].GetFloat());
        m_scene.camera.setViewport(m_width, m_height);
    } else {
        const glm::vec3 xyz1 = m_scene.xyz1, xyz2 = m_scene.xyz2;
        glm::vec3 target    = 0.5f * (xyz2 + xyz1);
        glm::vec3 eye       = target + 1.0f * (xyz2 - xyz1);
        glm::vec3 direction = glm::normalize(xyz2 - xyz1);
        glm::vec3 up        = {0.0f, 1.0f, 0.0f};
        if (std::abs(glm::dot(direction, up)) > 0.99f) {
            up = {0.0f, 0.0f, 1.0f};
        }
        m_scene.camera.setEye(eye);
        m_scene.camera.setTarget(target);
        m_scene.camera.setUp(up);
    }

    std::cout << m_scene.camera.getEye() << '\n';
    std::cout << m_scene.camera.getTarget() << '\n';
    std::cout << m_scene.xyz1 << ' ' << m_scene.xyz2 << '\n';

    // TODO: add environment map (IBL) support
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

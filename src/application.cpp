
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
    glfwWindowHint(GLFW_DEPTH_BITS, 24);  // allocate at least 24 depth bits for glfw swapbuffers, otherwise glenbale(GL_DEPTH_TEST) may not work
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
        nullptr
    );

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

        // Draw user interface
        drawUI(deltaTime);

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

void Application::applyTheme() {
    ImGuiStyle& style = ImGui::GetStyle();

    // 整体扁平化与圆角微调（参考图片中的轻微圆角与纯平线条）
    style.WindowRounding    = 2.0f;
    style.ChildRounding     = 2.0f;
    style.FrameRounding     = 3.0f;
    style.PopupRounding     = 2.0f;
    style.ScrollbarRounding = 4.0f;
    style.GrabRounding      = 3.0f;
    style.TabRounding       = 2.0f;

    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize  = 0.0f;
    style.PopupBorderSize  = 1.0f;

    ImVec4* colors = style.Colors;

    // 基础黑/灰背景色
    colors[ImGuiCol_Text]         = ImVec4(0.85f, 0.85f, 0.85f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);
    colors[ImGuiCol_WindowBg]     = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);  // 灰黑底色
    colors[ImGuiCol_ChildBg]      = ImVec4(0.09f, 0.09f, 0.09f, 1.00f);
    colors[ImGuiCol_PopupBg]      = ImVec4(0.14f, 0.14f, 0.14f, 0.95f);
    colors[ImGuiCol_Border]       = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

    // 输入框、滑块的基础颜色
    colors[ImGuiCol_FrameBg]        = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    colors[ImGuiCol_FrameBgActive]  = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);

    // 标题栏
    colors[ImGuiCol_TitleBg]          = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_TitleBgActive]    = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.10f, 0.10f, 0.10f, 0.70f);

    // 核心高亮：硬核橙色/琥珀色（模仿图片中的高亮色）
    ImVec4 orangeAccent  = ImVec4(0.72f, 0.34f, 0.12f, 1.00f);  // 主橙色
    ImVec4 orangeHovered = ImVec4(0.85f, 0.42f, 0.16f, 1.00f);  // 悬浮亮橙
    ImVec4 orangeActive  = ImVec4(0.60f, 0.28f, 0.09f, 1.00f);  // 按下深橙

    colors[ImGuiCol_Button]        = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = orangeHovered;
    colors[ImGuiCol_ButtonActive]  = orangeActive;

    // Header (TreeNode 选中、CollapsingHeader 展开)
    colors[ImGuiCol_Header]        = orangeAccent;
    colors[ImGuiCol_HeaderHovered] = orangeHovered;
    colors[ImGuiCol_HeaderActive]  = orangeActive;

    // CheckMark (复选框对勾)
    colors[ImGuiCol_CheckMark] = ImVec4(0.95f, 0.50f, 0.15f, 1.00f);

    // Slider
    colors[ImGuiCol_SliderGrab]       = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = orangeAccent;

    // Tabs
    colors[ImGuiCol_Tab]                = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_TabHovered]         = orangeHovered;
    colors[ImGuiCol_TabActive]          = orangeAccent;
    colors[ImGuiCol_TabUnfocused]       = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
}

void Application::drawUI(float deltaTime) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    drawSideBar(deltaTime);
    drawHUD(deltaTime);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Application::drawSideBar(float deltaTime) {
    // 移除窗口边框和内衬带来的缝隙样式，保持现代编辑器的扁平感
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));

    // =========================================================================
    // 1. ACTIVITY BAR (最左侧窄条，类似 VSCode 活动栏)
    // =========================================================================
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(m_activityBarWidth, static_cast<float>(m_height)), ImGuiCond_Always);

    ImGuiWindowFlags activityBarFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                                        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
                                        ImGuiWindowFlags_NoScrollbar;

    // 让活动条背景颜色稍微更深一点点
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.08f, 0.08f, 0.09f, 1.0f));

    if (ImGui::Begin("##ActivityBar", nullptr, activityBarFlags)) {
        auto drawTabButton = [this](const char* icon, const char* tooltip, SidebarTab tab) {
            bool isSelected = (m_currentTab == tab);

            // 如果被选中，高亮高反差显示按钮
            if (isSelected) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.25f, 1.0f));
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));  // 透明背景
            }

            // 绘制固定正方形的大图标按钮
            if (ImGui::Button(icon, ImVec2(m_activityBarWidth - 16, m_activityBarWidth - 16))) {
                if (isSelected) {
                    m_currentTab = SidebarTab::None;  // 如果已经是激活态，点击则“收起侧边栏”
                } else {
                    m_currentTab = tab;  // 切换到对应面板
                }
            }
            ImGui::PopStyleColor();

            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("%s", tooltip);
            }
            ImGui::Spacing();
        };

        // 渲染三个核心切换标签
        drawTabButton("⚙", "Renderer Settings", SidebarTab::Renderer);
        drawTabButton("💡", "Light Studio", SidebarTab::Lights);
        drawTabButton("📦", "Scene Entities", SidebarTab::Models);
    }
    ImGui::End();
    ImGui::PopStyleColor();  // 弹出 WindowBg 颜色

    // =========================================================================
    // 2. SIDEBAR PANEL (紧贴 Activity Bar 的详细参数面板)
    // =========================================================================
    if (m_currentTab != SidebarTab::None) {
        // 面板紧靠在 ActivityBar 的右侧 (x = m_activityBarWidth)
        ImGui::SetNextWindowPos(ImVec2(m_activityBarWidth, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(m_sideBarWidth, static_cast<float>(m_height)), ImGuiCond_Always);

        ImGuiWindowFlags panelFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                                      ImGuiWindowFlags_NoCollapse;

        // 根据当前选中的 Tab 赋予不同的 Window 标题
        std::string panelTitle = "Inspector";
        if (m_currentTab == SidebarTab::Renderer) panelTitle = "Renderer Pipeline";
        if (m_currentTab == SidebarTab::Lights) panelTitle = "Light Studio";
        if (m_currentTab == SidebarTab::Models) panelTitle = "Scene Entities";

        if (ImGui::Begin(panelTitle.c_str(), nullptr, panelFlags)) {
            // --- 分流渲染不同的控制面板 ---
            if (m_currentTab == SidebarTab::Renderer) {
                // 🛠️ 纯粹的渲染状态开关
                bool shadow = m_renderer.isShadowEnabled();
                if (ImGui::Checkbox("Shadow Mapping", &shadow)) m_renderer.enableShadow(shadow);

                bool ibl = m_renderer.isIBLEnabled();
                if (ImGui::Checkbox("Environment IBL", &ibl)) m_renderer.enableIBL(ibl);

                bool bloom = m_renderer.isBloomEnabled();
                if (ImGui::Checkbox("Bloom Blur", &bloom)) m_renderer.enableBloom(bloom);

                bool lensflare = m_renderer.isLensflareEnabled();
                if (ImGui::Checkbox("Lensflare Effect", &lensflare)) m_renderer.enableLensflare(lensflare);
            }

            else if (m_currentTab == SidebarTab::Lights) {
                // 💡 纯粹的光源集中控制列表
                auto& lights = m_scene.getLights();
                if (lights.empty()) {
                    ImGui::TextDisabled("No ambient/direct lights.");
                }

                // else {
                //     for (size_t i = 0; i < lights.size(); ++i) {
                //         if (!lights[i]) continue;
                //         if (ImGui::TreeNode(("Light Source [" + std::to_string(i) + "]").c_str())) {
                //             glm::vec3 position = lights[i]->getPosition();
                //             if (ImGui::DragFloat3("Position", &position.x, 0.1f)) lights[i]->setPosition(position);

                //             glm::vec3 color = lights[i]->getColor();
                //             if (ImGui::ColorEdit3("Color Spectrum", &color.x)) lights[i]->setColor(color);

                //             ImGui::TreePop();
                //         }
                //     }
                // }
            }

            else if (m_currentTab == SidebarTab::Models) {
                // 📦 纯粹的模型、相机和网格控制树
                if (ImGui::TreeNodeEx("Main Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
                    auto& camera = m_scene.getCamera();
                    if (camera) {
                        glm::vec3 eye = camera->getEye();
                        if (ImGui::SliderFloat3("Eye Pos", &eye.x, -50.0f, 50.0f, "%.2f")) camera->setEye(eye);
                        glm::vec3 target = camera->getTarget();
                        if (ImGui::SliderFloat3("Target Look", &target.x, -50.0f, 50.0f, "%.2f")) camera->setTarget(target);
                    }
                    ImGui::TreePop();
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                auto& models = m_scene.getModels();
                if (models.empty()) {
                    ImGui::TextDisabled("Empty world hierarchy.");
                }
                // else {
                //     ImGui::Text("Mesh Hierarchy:");
                //     for (size_t i = 0; i < models.size(); ++i) {
                //         if (!models[i]) continue;
                //         std::string nodeName = "📦 " + models[i]->getName();
                //         if (ImGui::TreeNode(nodeName.c_str())) {
                //             glm::vec3 position = models[i]->getPosition();
                //             if (ImGui::DragFloat3("Translate", &position.x, 0.05f)) models[i]->setPosition(position);

                //             glm::vec3 scale = models[i]->getScale();
                //             if (ImGui::DragFloat3("Scale", &scale.x, 0.01f, 0.001f, 10.0f)) models[i]->setScale(scale);

                //             ImGui::TreePop();
                //         }
                //     }
                // }
            }
        }
        ImGui::End();
    }

    ImGui::PopStyleVar(2);  // 弹出 WindowBorderSize 和 WindowPadding
}

void Application::drawHUD(float deltaTime) {
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
        ImGui::Text("Shadow Mapping : %s", m_renderer.isShadowEnabled() ? "On" : "Off");
        ImGui::Text("Environment IBL : %s", m_renderer.isIBLEnabled() ? "On" : "Off");
        ImGui::Text("Bloom Blur : %s", m_renderer.isBloomEnabled() ? "On" : "Off");
        ImGui::Text("Sreen Space Ambient Occlusion : %s", m_renderer.isSSAOEnabled() ? "On" : "Off");
        ImGui::Text("Temporal Anti-Aliasing : %s", m_renderer.isTAAEnabled() ? "On" : "Off");
    }
    ImGui::End();

    prevDrawCall = currDrawCall;
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
    if (!app) return;
    if (!app->m_mouseRightDragging && !app->m_mouseLeftDragging) return;

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
    if (!app) return;

    app->m_width  = width;
    app->m_height = height;
    app->m_renderer.setSize(width, height);
}

}  // namespace tinyglrenderer

#include "editor.hpp"

#include <string>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

namespace tinyglrenderer {

void Editor::setup() {
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
    colors[ImGuiCol_WindowBg]     = ImVec4(0.12f, 0.12f, 0.12f, 1.00f); // 灰黑底色
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
    ImVec4 orangeAccent  = ImVec4(0.72f, 0.34f, 0.12f, 1.00f); // 主橙色
    ImVec4 orangeHovered = ImVec4(0.85f, 0.42f, 0.16f, 1.00f); // 悬浮亮橙
    ImVec4 orangeActive  = ImVec4(0.60f, 0.28f, 0.09f, 1.00f); // 按下深橙

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

void Editor::shutdown() {
}

void Editor::draw(Scene& scene, RenderSetting& setting, const DisplayInfo& info) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    drawSideBar(scene, setting);
    drawHUD(scene, setting, info);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Editor::drawSideBar(Scene& scene, RenderSetting& setting) {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));

    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(m_activityBarWidth, static_cast<float>(setting.height)), ImGuiCond_Always);

    ImGuiWindowFlags activityBarFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                                        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
                                        ImGuiWindowFlags_NoScrollbar;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.08f, 0.08f, 0.09f, 1.0f));

    if (ImGui::Begin("##ActivityBar", nullptr, activityBarFlags)) {
        auto drawTabButton = [this](const char* icon, const char* tooltip, SidebarTab tab) {
            bool isSelected = (m_currTab == tab);
            if (isSelected) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.25f, 1.0f));
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
            }
            if (ImGui::Button(icon, ImVec2(m_activityBarWidth - 16, m_activityBarWidth - 16))) {
                if (isSelected) {
                    m_currTab = SidebarTab::TAB_NONE;
                } else {
                    m_currTab = tab;
                }
            }
            ImGui::PopStyleColor();

            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("%s", tooltip);
            }
            ImGui::Spacing();
        };

        drawTabButton("⚙", "Renderer Settings", SidebarTab::TAB_RENDERER);
        drawTabButton("💡", "Light Studio", SidebarTab::TAB_LIGHTS);
        drawTabButton("📦", "Scene Entities", SidebarTab::TAB_MODELS);
    }
    ImGui::End();
    ImGui::PopStyleColor();

    if (m_currTab != SidebarTab::TAB_NONE) {
        ImGui::SetNextWindowPos(ImVec2(m_activityBarWidth, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(m_sideBarWidth, static_cast<float>(setting.height)), ImGuiCond_Always);

        ImGuiWindowFlags panelFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                                      ImGuiWindowFlags_NoCollapse;

        std::string panelTitle = "Inspector";
        if (m_currTab == SidebarTab::TAB_RENDERER)
            panelTitle = "Renderer Pipeline";
        if (m_currTab == SidebarTab::TAB_LIGHTS)
            panelTitle = "Light Studio";
        if (m_currTab == SidebarTab::TAB_MODELS)
            panelTitle = "Scene Entities";

        if (ImGui::Begin(panelTitle.c_str(), nullptr, panelFlags)) {
            if (m_currTab == SidebarTab::TAB_RENDERER) {
                ImGui::Checkbox("Shadow Mapping", &setting.shadow);
                ImGui::Checkbox("Environment IBL", &setting.ibl);
                ImGui::Checkbox("Bloom Blur", &setting.bloom);
                ImGui::Checkbox("Lensflare Effect", &setting.lensflare);
            } else if (m_currTab == SidebarTab::TAB_LIGHTS) {
                // 💡 纯粹的光源集中控制列表
                auto& lights = scene.getLights();
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
            } else if (m_currTab == SidebarTab::TAB_MODELS) {
                if (ImGui::TreeNodeEx("Main Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
                    auto& camera = scene.getCamera();
                    if (camera) {
                        glm::vec3 eye = camera->getEye();
                        if (ImGui::SliderFloat3("Eye Pos", &eye.x, -50.0f, 50.0f, "%.2f"))
                            camera->setEye(eye);
                        glm::vec3 target = camera->getTarget();
                        if (ImGui::SliderFloat3("Target Look", &target.x, -50.0f, 50.0f, "%.2f"))
                            camera->setTarget(target);
                    }
                    ImGui::TreePop();
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                auto& models = scene.getModels();
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

    ImGui::PopStyleVar(2); // 弹出 WindowBorderSize 和 WindowPadding
}

void Editor::drawResource(Scene& scene, RenderSetting& setting) {
}

void Editor::drawHUD(Scene& scene, RenderSetting& setting, const DisplayInfo& info) {
    const float padding   = 10.0f;
    ImVec2 windowPos      = ImVec2(static_cast<float>(setting.width) - padding, padding);
    ImVec2 windowPosPivot = ImVec2(1.0f, 0.0f); // right top corner (1.0, 0.0)

    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always, windowPosPivot);
    ImGui::SetNextWindowBgAlpha(0.35f); // slightly transparent black background, not blocking 3D scene

    // Make the window invisible and not interactable with the user
    ImGuiWindowFlags windowFlags =
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav |
        ImGuiWindowFlags_NoMove;

    static uint32_t prevDrawCall = 0;
    uint32_t currDrawCall        = info.drawCall;

    if (ImGui::Begin("HUD", nullptr, windowFlags)) {
        ImGui::Text("PERFORMANCE");
        ImGui::Separator();
        ImGui::Text("FPS       : %.1f", info.fps);
        ImGui::Text("Frame Time: %.2f ms", info.deltaTime * 1000.0f);
        ImGui::Text("Draw Call: %d draw calls", currDrawCall - prevDrawCall);

        ImGui::Spacing();
        ImGui::Text("STATE");
        ImGui::Separator();
        const auto& camera = scene.getCamera();
        glm::vec3 pos      = camera->getEye();
        glm::vec3 target   = camera->getTarget();
        glm::vec3 front    = glm::normalize(target - pos);
        ImGui::Text("Camera Position : (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);
        ImGui::Text("Camera FrontVec : (%.2f, %.2f, %.2f)", front.x, front.y, front.z);
        ImGui::Text("Shadow Mapping : %s", setting.shadow ? "On" : "Off");
        ImGui::Text("Environment IBL : %s", setting.ibl ? "On" : "Off");
        ImGui::Text("Bloom Blur : %s", setting.bloom ? "On" : "Off");
        ImGui::Text("Lensflare : %s", setting.lensflare ? "On" : "Off");
    }
    ImGui::End();

    prevDrawCall = currDrawCall;
}

}; // namespace tinyglrenderer

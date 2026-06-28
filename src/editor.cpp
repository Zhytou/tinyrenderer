#include "editor.hpp"

#include <string>

#include <icon/IconsFontAwesome6.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

namespace tinyglrenderer {

void Editor::setup() {
    // ==========================================
    // Load base font and icon font
    // ==========================================
    auto& io                           = ImGui::GetIO();
    static const ImWchar* glyph_ranges = io.Fonts->GetGlyphRangesDefault();
    static const ImWchar icon_ranges[] = {0xf000, 0xf3ff, 0}; // range of FontAwesome6

    m_fonts["text"]       = io.Fonts->AddFontFromFileTTF("../asset/fonts/DejaVuSans.ttf", m_setting.fontSize, nullptr, glyph_ranges);
    m_fonts["small_text"] = io.Fonts->AddFontFromFileTTF("../asset/fonts/DejaVuSans.ttf", m_setting.smallFontSize, nullptr, glyph_ranges);
    m_fonts["large_text"] = io.Fonts->AddFontFromFileTTF("../asset/fonts/DejaVuSans.ttf", m_setting.largeFontSize, nullptr, glyph_ranges);
    m_fonts["icon"]       = io.Fonts->AddFontFromFileTTF("../asset/fonts/fa-solid-900.ttf", m_setting.iconSize, nullptr, icon_ranges);
    m_fonts["small_icon"] = io.Fonts->AddFontFromFileTTF("../asset/fonts/fa-solid-900.ttf", m_setting.smallIconSize, nullptr, icon_ranges);
    m_fonts["large_icon"] = io.Fonts->AddFontFromFileTTF("../asset/fonts/fa-solid-900.ttf", m_setting.largeIconSize, nullptr, icon_ranges);

    io.FontDefault = m_fonts["text"];

    // ==========================================
    // Config ImGui style
    // ==========================================
    ImGuiStyle& style = ImGui::GetStyle();
    // rounding
    style.WindowRounding    = 0.0f;
    style.ChildRounding     = 0.0f;
    style.FrameRounding     = 2.0f;
    style.PopupRounding     = 2.0f;
    style.ScrollbarRounding = 3.0f;
    style.GrabRounding      = 2.0f;
    style.TabRounding       = 0.0f;
    // border size
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize  = 0.0f;
    style.PopupBorderSize  = 1.0f;
    // spacing
    style.ItemSpacing      = ImVec2(6.0f, 6.0f);
    style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);

    // ==========================================
    // Config ImGui background colors
    // ==========================================
    ImVec4* colors                = style.Colors;
    colors[ImGuiCol_Text]         = ImVec4(0.88f, 0.88f, 0.92f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.40f, 0.40f, 0.45f, 1.00f);
    colors[ImGuiCol_WindowBg]     = ImVec4(0.11f, 0.11f, 0.13f, 1.00f);
    colors[ImGuiCol_ChildBg]      = ImVec4(0.08f, 0.08f, 0.10f, 1.00f);
    colors[ImGuiCol_PopupBg]      = ImVec4(0.14f, 0.14f, 0.16f, 0.98f);
    colors[ImGuiCol_Border]       = ImVec4(0.18f, 0.18f, 0.22f, 1.00f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    // frame background
    colors[ImGuiCol_FrameBg]        = ImVec4(0.16f, 0.16f, 0.19f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.22f, 0.22f, 0.26f, 1.00f);
    colors[ImGuiCol_FrameBgActive]  = ImVec4(0.26f, 0.26f, 0.32f, 1.00f);
    // title background
    colors[ImGuiCol_TitleBg]          = ImVec4(0.09f, 0.09f, 0.11f, 1.00f);
    colors[ImGuiCol_TitleBgActive]    = ImVec4(0.11f, 0.11f, 0.13f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.09f, 0.09f, 0.11f, 0.70f);

    // ==========================================
    // Config theme colors
    // ==========================================
    ImVec4 bg_window, bg_child, bg_frame, bg_title, text_normal;
    ImVec4 accent, hovered, active, header_select;
    switch (m_setting.currTheme) {
        case EditorTheme::PURPLE_THEME: { // 1. VSCode Dark+ (默认经典暗紫/深灰)
            bg_window     = ImVec4(0.11f, 0.11f, 0.13f, 1.00f);
            bg_child      = ImVec4(0.08f, 0.08f, 0.10f, 1.00f);
            bg_frame      = ImVec4(0.16f, 0.16f, 0.19f, 1.00f);
            bg_title      = ImVec4(0.09f, 0.09f, 0.11f, 1.00f);
            text_normal   = ImVec4(0.88f, 0.88f, 0.92f, 1.00f);
            accent        = ImVec4(0.41f, 0.26f, 0.65f, 1.00f); // 现代紫
            hovered       = ImVec4(0.50f, 0.33f, 0.78f, 1.00f);
            active        = ImVec4(0.31f, 0.18f, 0.50f, 1.00f);
            header_select = ImVec4(0.26f, 0.20f, 0.38f, 1.00f);
        } break;
        case EditorTheme::PINK_THEME: {                         // 2. One Dark Pro (VSCode 流行榜首，优雅冷蓝/高级极客)
            bg_window     = ImVec4(0.16f, 0.17f, 0.20f, 1.00f); // 带有微弱蓝绿相间的深灰
            bg_child      = ImVec4(0.13f, 0.14f, 0.16f, 1.00f);
            bg_frame      = ImVec4(0.21f, 0.23f, 0.27f, 1.00f);
            bg_title      = ImVec4(0.11f, 0.12f, 0.14f, 1.00f);
            text_normal   = ImVec4(0.67f, 0.72f, 0.80f, 1.00f); // 柔和的粉蓝白
            accent        = ImVec4(0.38f, 0.53f, 0.80f, 1.00f); // 标志性 OneDark 科技蓝
            hovered       = ImVec4(0.48f, 0.65f, 0.93f, 1.00f);
            active        = ImVec4(0.28f, 0.41f, 0.68f, 1.00f);
            header_select = ImVec4(0.22f, 0.28f, 0.39f, 1.00f);
        } break;
        case EditorTheme::DRACULA_THEME: {                      // 3. Dracula (吸血鬼主题，高饱和度赛博霓虹粉/绿)
            bg_window     = ImVec4(0.16f, 0.16f, 0.21f, 1.00f); // 标志性冷黛蓝底
            bg_child      = ImVec4(0.11f, 0.11f, 0.16f, 1.00f);
            bg_frame      = ImVec4(0.24f, 0.24f, 0.33f, 1.00f);
            bg_title      = ImVec4(0.13f, 0.13f, 0.18f, 1.00f);
            text_normal   = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
            accent        = ImVec4(0.74f, 0.49f, 0.86f, 1.00f); // 经典的蕾丝魅惑紫
            hovered       = ImVec4(1.00f, 0.49f, 0.77f, 1.00f); // 悬浮变身吸血鬼粉红
            active        = ImVec4(0.54f, 0.35f, 0.66f, 1.00f);
            header_select = ImVec4(0.27f, 0.26f, 0.44f, 1.00f);
        } break;
        case EditorTheme::TOKYO_THEME: {                        // 4. Tokyo Night (东京暗夜，深邃幽蓝与荧光青绿色)
            bg_window     = ImVec4(0.10f, 0.11f, 0.15f, 1.00f); // 极具质感的深邃星空底
            bg_child      = ImVec4(0.07f, 0.08f, 0.11f, 1.00f);
            bg_frame      = ImVec4(0.15f, 0.17f, 0.24f, 1.00f);
            bg_title      = ImVec4(0.09f, 0.09f, 0.13f, 1.00f);
            text_normal   = ImVec4(0.78f, 0.82f, 0.93f, 1.00f);
            accent        = ImVec4(0.26f, 0.65f, 0.93f, 1.00f); // 冰蓝色
            hovered       = ImVec4(0.44f, 0.83f, 0.81f, 1.00f); // 悬浮变成高亮荧光青
            active        = ImVec4(0.18f, 0.48f, 0.71f, 1.00f);
            header_select = ImVec4(0.18f, 0.24f, 0.35f, 1.00f);
        } break;
        default: {                                              // 5. Monokai Pro (经典老牌黑客，岩石枯灰与焦糖金黄)
            bg_window     = ImVec4(0.16f, 0.16f, 0.16f, 1.00f); // 纯正炭黑岩石质感
            bg_child      = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
            bg_frame      = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
            bg_title      = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
            text_normal   = ImVec4(0.93f, 0.93f, 0.92f, 1.00f);
            accent        = ImVec4(0.93f, 0.72f, 0.38f, 1.00f); // Monokai 标志性浅焦糖金黄
            hovered       = ImVec4(0.89f, 0.41f, 0.47f, 1.00f); // 悬浮触发高级哑光红
            active        = ImVec4(0.73f, 0.55f, 0.26f, 1.00f);
            header_select = ImVec4(0.27f, 0.24f, 0.20f, 1.00f);
        } break;
    }

    // ==========================================
    // Apply layout backgrounds to ImGui
    // ==========================================
    colors[ImGuiCol_Text]         = text_normal;
    colors[ImGuiCol_TextDisabled] = ImVec4(text_normal.x * 0.5f, text_normal.y * 0.5f, text_normal.z * 0.5f, 1.00f);
    colors[ImGuiCol_WindowBg]     = bg_window;
    colors[ImGuiCol_ChildBg]      = bg_child;
    colors[ImGuiCol_PopupBg]      = ImVec4(bg_window.x + 0.03f, bg_window.y + 0.03f, bg_window.z + 0.03f, 0.98f);
    colors[ImGuiCol_Border]       = ImVec4(bg_window.x + 0.07f, bg_window.y + 0.07f, bg_window.z + 0.09f, 1.00f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

    colors[ImGuiCol_FrameBg]        = bg_frame;
    colors[ImGuiCol_FrameBgHovered] = ImVec4(bg_frame.x + 0.06f, bg_frame.y + 0.06f, bg_frame.z + 0.07f, 1.00f);
    colors[ImGuiCol_FrameBgActive]  = ImVec4(bg_frame.x + 0.10f, bg_frame.y + 0.10f, bg_frame.z + 0.13f, 1.00f);

    colors[ImGuiCol_TitleBg]          = bg_title;
    colors[ImGuiCol_TitleBgActive]    = bg_window;
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(bg_title.x, bg_title.y, bg_title.z, 0.70f);

    // ==========================================
    // Apply reactive components theme colors
    // ==========================================
    // button
    colors[ImGuiCol_Button]        = bg_frame;
    colors[ImGuiCol_ButtonHovered] = hovered;
    colors[ImGuiCol_ButtonActive]  = active;
    // tree node
    colors[ImGuiCol_Header]        = header_select;
    colors[ImGuiCol_HeaderHovered] = accent;
    colors[ImGuiCol_HeaderActive]  = active;
    // checkbox
    colors[ImGuiCol_CheckMark] = (m_setting.currTheme == EditorTheme::DEFAULT_THEME) ? hovered : accent;
    // slider
    colors[ImGuiCol_SliderGrab]       = ImVec4(bg_frame.x + 0.15f, bg_frame.y + 0.15f, bg_frame.z + 0.18f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = accent;
    // tabs
    colors[ImGuiCol_Tab]                = bg_title;
    colors[ImGuiCol_TabHovered]         = hovered;
    colors[ImGuiCol_TabActive]          = bg_window;
    colors[ImGuiCol_TabUnfocused]       = bg_title;
    colors[ImGuiCol_TabUnfocusedActive] = bg_child;
    // scrollbar
    colors[ImGuiCol_ScrollbarBg]          = bg_child;
    colors[ImGuiCol_ScrollbarGrab]        = ImVec4(bg_frame.x + 0.08f, bg_frame.y + 0.08f, bg_frame.z + 0.10f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(bg_frame.x + 0.16f, bg_frame.y + 0.16f, bg_frame.z + 0.20f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]  = accent;
}

void Editor::shutdown() {
}

void Editor::draw(Scene& scene, const DisplayInfo& info) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    drawCustomTitleBar();
    drawResourcePanel(scene);
    drawSideBar(scene);
    drawHUD(scene, info);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

std::string Editor::hover(float x, float y) {
    const bool sb_active  = m_setting.currSBTab != SideBarTab::SB_TAB_NONE;
    const float width     = m_setting.width;
    const float height    = m_setting.height;
    const float tb_height = m_setting.titleBarHeight;
    const float ab_width  = m_setting.activityBarWidth;
    const float ab_height = height - tb_height;
    const float sb_width  = m_setting.sideBarWidth;
    const float sb_height = ab_height;
    const float rw_height = sb_height;

    std::unordered_map<std::string, glm::vec4> name2Rects = {
        {"titlebar", {0.0f, 0.0f, width, tb_height}},
        {"activitybar", {0.0f, 0.0f, ab_width, ab_height}},
        {"sidebar", sb_active ? glm::vec4(0.0f, 0.0f, -1.0f, -1.0f) : glm::vec4(ab_width, 0.0f, sb_width, sb_height)},
        {"renderwindow", sb_active ? glm::vec4(ab_width, 0.0f, width - ab_width, rw_height) : glm::vec4(ab_width + sb_width, 0.0f, width - ab_width - sb_width, rw_height)}
    };

    for (auto& [name, rect] : name2Rects) { // rect.w is height
        if (rect.x <= x && x <= rect.x + rect.w && rect.y <= y && y <= rect.y + rect.w) {
            return name;
        }
    }
    return "none";
}

void Editor::drawCustomTitleBar() {
    bool maximized       = m_funcs["titlebar_is_maximized"]();
    float buttonWidth    = 45.0f; // button width of title bar is hard coded
    const int numButtons = 3;
    auto label2Callback  = std::vector<std::pair<std::string, std::string>>{
        {std::string(ICON_FA_MINUS) + std::string("##TitleBar_Minimize"),                                       "titlebar_minimize"        },
        {std::string(maximized ? ICON_FA_WINDOW_RESTORE : ICON_FA_SQUARE) + std::string("##TitleBar_Maximize"), "titlebar_maximize_restore"},
        {std::string(ICON_FA_XMARK) + std::string("##TitleBar_Close"),                                          "titlebar_close"           },
    };
    const ImVec2 titleBarPos  = ImVec2(0.0f, 0.0f);
    const ImVec2 titleBarSize = ImVec2(m_setting.width, m_setting.titleBarHeight);
    const ImVec2 buttonSize   = ImVec2(buttonWidth, m_setting.titleBarHeight);
    const ImVec2 padding      = ImVec2(m_setting.padding, 0.0f);
    const float borderSize    = m_setting.borderSize;
    const float textHeight    = ImGui::GetTextLineHeight();
    auto flags                = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar;

    ImGui::SetNextWindowPos(titleBarPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(titleBarSize, ImGuiCond_Always);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, padding);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, borderSize);
    if (ImGui::Begin("##CustomTitleBar", nullptr, flags)) {
        ImGui::SetCursorPosY((titleBarSize.y - textHeight) * 0.5f); // set cursor position for later text draw
        ImGui::Text("TinyGLRenderer");
        ImGui::SetCursorPosY(0.0f);                                               // reset cursor position
        ImGui::SetCursorPosX(titleBarSize.x - (buttonWidth * numButtons) - 1.0f); // set cursor position for later button draw

        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);       // no rounding for title bar buttons
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0)); // no spacing between buttons
        ImGui::PushFont(m_fonts["small_icon"]);
        for (auto& [buttonLabel, buttonCallback] : label2Callback) {
            if (buttonCallback == "titlebar_close") {
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.90f, 0.20f, 0.20f, 1.0f)); // when hovered on close button, turn it into a red X
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.70f, 0.15f, 0.15f, 1.0f));  // when clicked on close button, turn it into a darker red X
            }
            if (ImGui::Button(buttonLabel.c_str(), buttonSize)) {
                m_funcs[buttonCallback]();
            }
            if (buttonCallback == "titlebar_close") {
                ImGui::PopStyleColor(2);
            }
            ImGui::SameLine();
        }
        ImGui::PopFont();
        ImGui::PopStyleVar(2); // remove temporary rounding and spacing style
    }

    ImGui::End();
    ImGui::PopStyleVar(2); // 弹出 WindowPadding 和 WindowBorderSize
}

void Editor::drawSideBar(Scene& scene) {
    ImGuiStyle& style      = ImGui::GetStyle();
    const float borderSize = m_setting.borderSize;
    ImVec2 activityBarPos  = ImVec2(0.0f, m_setting.titleBarHeight);
    ImVec2 activityBarSize = ImVec2(m_setting.activityBarWidth, m_setting.height - m_setting.titleBarHeight);
    ImVec2 buttonSize      = ImVec2(m_setting.activityBarWidth, m_setting.activityBarWidth);
    ImVec2 sideBarPos      = ImVec2(m_setting.activityBarWidth, m_setting.titleBarHeight);
    ImVec2 sideBarSize     = ImVec2(m_setting.sideBarWidth, m_setting.height - m_setting.titleBarHeight);
    auto sideBarTab2Name   = std::unordered_map<SideBarTab, const char*>{
        {SideBarTab::SB_TAB_NONE,     "Inspector##SideBarTab_None"            },
        {SideBarTab::SB_TAB_RENDERER, "Renderer Settings##SideBarTab_Renderer"},
        {SideBarTab::SB_TAB_LIGHTS,   "Light Studio##SideBarTab_Lights"       },
        {SideBarTab::SB_TAB_MODELS,   "Scene Entities##SideBarTab_Models"     },
    };
    auto sideBarTitle     = sideBarTab2Name[m_setting.currSBTab];
    auto activityBarFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar;
    auto sideBarFlags     = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;

    ImGui::SetNextWindowPos(activityBarPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(activityBarSize, ImGuiCond_Always);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, borderSize);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, style.Colors[ImGuiCol_ChildBg]);
    if (ImGui::Begin("##ActivityBar", nullptr, activityBarFlags)) {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f)); // remove spacing between buttons
        drawSideBarButton(ICON_FA_GEAR, "Renderer Settings", SideBarTab::SB_TAB_RENDERER, buttonSize);
        drawSideBarButton(ICON_FA_LIGHTBULB, "Light Studio", SideBarTab::SB_TAB_LIGHTS, buttonSize);
        drawSideBarButton(ICON_FA_CUBE, "Scene Entities", SideBarTab::SB_TAB_MODELS, buttonSize);
        ImGui::PopStyleVar(1); // pop out ItemSpacing
    }
    ImGui::End();
    ImGui::PopStyleColor();
    if (m_setting.currSBTab == SideBarTab::SB_TAB_NONE) {
        ImGui::PopStyleVar(2);
        return;
    }

    ImGui::SetNextWindowPos(sideBarPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(sideBarSize, ImGuiCond_Always);
    if (ImGui::Begin(sideBarTitle, nullptr, sideBarFlags)) {
        if (m_setting.currSBTab == SideBarTab::SB_TAB_RENDERER) {
            ImGui::Checkbox("Shadow Mapping", &m_rendererSetting.shadow);
            ImGui::Checkbox("Environment IBL", &m_rendererSetting.ibl);
            ImGui::Checkbox("Bloom Blur", &m_rendererSetting.bloom);
            ImGui::Checkbox("Lensflare Effect", &m_rendererSetting.lensflare);
        } else if (m_setting.currSBTab == SideBarTab::SB_TAB_LIGHTS) {
            auto& lights = scene.getLights();
            if (lights.empty()) {
                ImGui::TextDisabled("No ambient/direct lights.");
            }
        } else if (m_setting.currSBTab == SideBarTab::SB_TAB_MODELS) {
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
        }
    }
    ImGui::End();

    ImGui::PopStyleVar(2); // 弹出 WindowBorderSize 和 WindowPadding
}

void Editor::drawSideBarButton(const std::string& icon, const std::string& tooltip, SideBarTab tab, const ImVec2& size) {
    ImGuiStyle& style = ImGui::GetStyle();
    std::string label = icon + "##SideActivityBar_" + std::to_string(static_cast<int>(tab));

    bool selected = (m_setting.currSBTab == tab);
    if (selected) { ImGui::PushStyleColor(ImGuiCol_Button, style.Colors[ImGuiCol_Header]); }
    ImGui::PushFont(m_fonts["large_icon"]); // use icon font to render icon
    if (ImGui::Button(label.c_str(), size)) {
        m_setting.currSBTab = (selected) ? SideBarTab::SB_TAB_NONE : tab;
    }
    ImGui::PopFont(); // pop out icon font to restore default font
    if (selected) { ImGui::PopStyleColor(); }

    if (ImGui::IsItemHovered()) { // show tooltip when cursor hover on the button
        ImGui::SetTooltip("%s", tooltip.c_str());
    }
}

void Editor::drawResourcePanel(Scene& scene) {
    float resourcePanelPosX    = (m_setting.currSBTab == SideBarTab::SB_TAB_NONE ? 0 : m_setting.sideBarWidth) + m_setting.activityBarWidth;
    float resourcePanelPosY    = m_setting.height - m_setting.resourcePanelHeight;
    ImVec2 resourcePanelSize   = ImVec2(m_setting.width - resourcePanelPosX, m_setting.resourcePanelHeight);
    auto resourcePanelTab2Name = std::unordered_map<ResourcePanelTab, const char*>{
        {ResourcePanelTab::RP_TAB_MESHES,   "Loaded Meshes##ResourcePanelTab_Meshes"    },
        {ResourcePanelTab::RP_TAB_SHADERS,  "Loaded Shaders##ResourcePanelTab_SHADERS"  },
        {ResourcePanelTab::RP_TAB_TEXTURES, "Loaded Textures##ResourcePanelTab_TEXTURES"},
    };
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar;

    ImGui::SetNextWindowPos(ImVec2(resourcePanelPosX, resourcePanelPosY));
    ImGui::SetNextWindowSize(resourcePanelSize);
    ImGui::Begin("##ResourcePanel", nullptr, flags);

    // =========================================================================
    // Draw Resource Panel Tabs
    // =========================================================================
    for (auto& [tab, name] : resourcePanelTab2Name) {
        bool selected = (m_setting.currRPTab == tab);
        if (selected) { ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_Header]); }
        if (ImGui::Button(name)) {
            m_setting.currRPTab = tab;
        }
        if (selected) { ImGui::PopStyleColor(); }
        ImGui::SameLine();
    }
    ImGui::Separator();

    // =========================================================================
    // Split the resource panel into two parts: left list (width40%) and right details (width70%)
    // =========================================================================
    float innerPanelWidth  = ImGui::GetContentRegionAvail().x;
    float innerPanelHeight = ImGui::GetContentRegionAvail().y;

    // =========================================================================
    // Draw left list with 40% width for all tabs
    // =========================================================================
    ImGui::BeginChild("##LeftList", ImVec2(innerPanelWidth * 0.4f, innerPanelHeight), true);
    int selectedItemIdx = 0;
    std::vector<std::string> items;
    // if (selectedItemIdx >= items.size())
    //     selectedItemIdx = 0;
    // for (int i = 0; i < items.size(); ++i) {
    //     const bool isSelected = (selectedItemIdx == i);
    //     if (ImGui::Selectable(items[i].c_str(), isSelected)) {
    //         selectedItemIdx = i;
    //     }
    // }
    ImGui::EndChild();

    ImGui::SameLine();

    // =========================================================================
    // 第二栏：中部详细信息（占剩余宽度的 70%）
    // =========================================================================
    float remainingWidth = ImGui::GetContentRegionAvail().x;
    ImGui::BeginChild("##MiddleDetails", ImVec2(remainingWidth * 0.7f, innerPanelHeight), true);

    if (!items.empty() && selectedItemIdx < items.size()) {
        std::string currentName = items[selectedItemIdx];

        // 打印元数据 (完美对齐图片格式)
        ImGui::Text("name: %s", currentName.c_str());

        if (m_setting.currRPTab == ResourcePanelTab::RP_TAB_MESHES) { // 模型详情
            int meshCount = (currentName == "nanosuit.obj") ? 7 : 1;
            int texCount  = (currentName == "nanosuit.obj") ? 19 : 2;
            ImGui::Text("mesh count: %d", meshCount);
            ImGui::Text("ref count: 1");
            ImGui::Text("loaded textures: %d", texCount);
            ImGui::Text("directory: assets/models/%s", currentName.substr(0, currentName.find_last_of('.')).c_str());
        } else if (m_setting.currRPTab == ResourcePanelTab::RP_TAB_TEXTURES) { // 纹理详情
            ImGui::Text("ref count: 1");
            ImGui::Text("path: assets/textures/%s", currentName.c_str());

            // 模仿下拉格式选择框
            static int comboType   = 0;
            const char* texTypes[] = {"SRGB", "RGBA", "RGB", "RED"};
            ImGui::SetNextItemWidth(150.0f);
            ImGui::Combo("type", &comboType, texTypes, IM_ARRAYSIZE(texTypes));

            ImGui::Text("width: 1024");
            ImGui::Text("height: 1024");
        } else { // Shader 详情
            ImGui::Text("ref count: 2");
            ImGui::Text("stage: Vertex & Fragment");
        }
    }
    ImGui::EndChild();

    ImGui::SameLine();

    // =========================================================================
    // 第三栏：右侧预览区域（占满剩余空间）
    // =========================================================================
    ImGui::BeginChild("##RightPreview", ImVec2(0, innerPanelHeight), true);

    if (m_setting.currRPTab == ResourcePanelTab::RP_TAB_TEXTURES && !items.empty()) {
        // 如果是纹理标签，绘制实时预览图
        // 实际开发中，这里应当通过名字找到你的 Texture 对象，拿到它的 OpenGL Texture ID
        // ImTextureID texID = (ImTextureID)(intptr_t)texture->getID();

        // 此处暂用 ImGui 纯色绘制一个 128x128 占位矩形（或类似图片中的火警栓红色纹理效果）
        ImVec2 uv0       = ImVec2(0.0f, 0.0f);
        ImVec2 uv1       = ImVec2(1.0f, 1.0f);
        ImVec4 tintColor = ImVec4(0.5f, 0.1f, 0.1f, 1.0f); // 类似 hydrant_albedo.jpg 的暗红底色

        // 如果没有绑定的真实纹理 ID，可以用内置 ImGui 绘制
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 pMin          = ImGui::GetCursorScreenPos();
        float previewSize    = 0; //ImMin(ImGui::GetContentRegionAvail().x, childHeight - 10.0f);
        ImVec2 pMax          = ImVec2(pMin.x + previewSize, pMin.y + previewSize);

        drawList->AddRectFilled(pMin, pMax, IM_COL32(130, 20, 20, 255)); // 模拟红色材质
        drawList->AddRect(pMin, pMax, IM_COL32(255, 255, 255, 50));      // 微弱白边
    } else {
        // 模型或 Shader 展示一个扁平化的小图标底色
        ImGui::TextUnformatted("[ No Preview ]");
    }

    ImGui::EndChild();

    ImGui::End();
}

void Editor::drawHUD(Scene& scene, const DisplayInfo& info) {
    static uint32_t prevDrawCall = 0;
    uint32_t currDrawCall        = info.drawCall;
    const float width            = m_setting.width;
    const float padding          = m_setting.padding;
    ImVec2 hudPos                = ImVec2(width - padding, padding);
    ImVec2 hudPosPivot           = ImVec2(1.0f, 0.0f);
    ImGuiWindowFlags flags       = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove; // set the window unavailable to interact with the user

    ImGui::SetNextWindowPos(hudPos, ImGuiCond_Always, hudPos); // set the window position to the top right corner of the screen
    ImGui::SetNextWindowBgAlpha(0.35f);                        // slightly transparent black background, not blocking 3D scene
    if (ImGui::Begin("HUD", nullptr, flags)) {
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
        ImGui::Text("Shadow Mapping : %s", m_rendererSetting.shadow ? "On" : "Off");
        ImGui::Text("Environment IBL : %s", m_rendererSetting.ibl ? "On" : "Off");
        ImGui::Text("Bloom Blur : %s", m_rendererSetting.bloom ? "On" : "Off");
        ImGui::Text("Lensflare : %s", m_rendererSetting.lensflare ? "On" : "Off");
    }
    ImGui::End();

    prevDrawCall = currDrawCall;
}

}; // namespace tinyglrenderer

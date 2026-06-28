#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <imgui/imgui.h>

#include <functional>
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <unordered_map>

#include "displayinfo.hpp"
#include "editorsetting.hpp"
#include "renderersetting.hpp"
#include "scene.hpp"

namespace tinyglrenderer {

class Editor {
   public:
    Editor(EditorSetting& editorSetting, RendererSetting& rendererSetting) : m_setting(editorSetting), m_rendererSetting(rendererSetting) {}
    ~Editor() { shutdown(); }

    Editor(const Editor&)            = delete;
    Editor& operator=(const Editor&) = delete;

    void setup();
    void shutdown();

    // draw the editor window
    void draw(Scene& scene, const DisplayInfo& info);
    // return the hovered editor widget name(sidebar/titlebar/resourcebrowser/renderer)
    std::string hover(float x, float y);

    void setCallback(const std::string& name, std::function<bool()> func) {
        m_funcs[name] = func;
    }

   private:
    void drawCustomTitleBar();
    void drawSideBar(Scene& scene);
    void drawResourcePanel(Scene& scene);
    void drawHUD(Scene& scene, const DisplayInfo& info);

    void drawSideBarButton(const std::string& icon, const std::string& tooltip, SideBarTab tab, const ImVec2& size);

    std::unordered_map<std::string, std::function<bool()>> m_funcs;
    std::unordered_map<std::string, ImFont*> m_fonts; // no need RAII, ImGui::DestroyContext() is called in Application::shutdown()

    EditorSetting& m_setting;
    RendererSetting& m_rendererSetting;
};

} // namespace tinyglrenderer
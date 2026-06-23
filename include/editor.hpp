#pragma once

#include <memory>
#include <string>

#include "displayinfo.hpp"
#include "rendersetting.hpp"
#include "scene.hpp"

namespace tinyglrenderer {

enum class SidebarTab {
    TAB_NONE,
    TAB_RENDERER = 0,
    TAB_LIGHTS,
    TAB_MODELS,
};

class Editor {
   public:
    Editor()                         = default;
    ~Editor()                        = default;
    Editor(const Editor&)            = delete;
    Editor& operator=(const Editor&) = delete;

    void setup();
    void shutdown();

    void draw(Scene& scene, RenderSetting& setting, const DisplayInfo& info);

   private:
    void drawSideBar(Scene& scene, RenderSetting& setting);
    void drawResource(Scene& scene, RenderSetting& setting);
    void drawHUD(Scene& scene, RenderSetting& setting, const DisplayInfo& info);

    SidebarTab m_currTab           = SidebarTab::TAB_NONE;
    const float m_activityBarWidth = 30.0f;
    const float m_sideBarWidth     = 350.0f;
};

} // namespace tinyglrenderer
#pragma once

namespace tinyglrenderer {

enum class SideBarTab {
    SB_TAB_NONE = 0,
    SB_TAB_RENDERER,
    SB_TAB_LIGHTS,
    SB_TAB_MODELS,
};

enum class ResourcePanelTab {
    RP_TAB_MESHES = 0,
    RP_TAB_SHADERS,
    RP_TAB_TEXTURES,
};

enum class EditorTheme {
    PURPLE_THEME  = 1,
    PINK_THEME    = 2,
    DRACULA_THEME = 3,
    TOKYO_THEME   = 4,
    DEFAULT_THEME = 5,
};

struct EditorSetting {
    SideBarTab currSBTab       = SideBarTab::SB_TAB_NONE;
    ResourcePanelTab currRPTab = ResourcePanelTab::RP_TAB_MESHES;
    EditorTheme currTheme      = EditorTheme::PURPLE_THEME;

    int currRPItemIndex = 0; // Current selected item index in the left list of resource panel

    float width      = 1280.0f;
    float height     = 720.0f;
    float padding    = 10.0f;
    float borderSize = 1.0f;

    const float titleBarHeight      = 38.0f;
    const float activityBarWidth    = 65.0f;
    const float sideBarWidth        = 180.0f;
    const float resourcePanelHeight = 300.0f;

    const float smallFontSize = 14.0f;
    const float fontSize      = 22.0f;
    const float largeFontSize = 34.0f;
    const float smallIconSize = 20.0f;
    const float iconSize      = 32.0f;
    const float largeIconSize = 40.0f;
};

} // namespace tinyglrenderer
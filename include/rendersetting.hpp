#pragma once

#include <cstdint>

namespace tinyglrenderer {

struct RenderSetting {
    bool deferred  = false; // deferred rendering enabled or not
    bool ibl       = false; // image based light enabled or not
    bool shadow    = false; // shadow mapping enabled or not
    bool bloom     = true;  // bloom blur enabled or not
    bool lensflare = true;  // lensflare enabled or not
    bool dirtmask  = true;  // dirtmask enabled or not
    bool ssao      = false; // screen space ambient occlusion enabled or not
    bool taa       = false; // temporal anti aliasing enabled or not

    uint32_t x                  = 0;
    uint32_t y                  = 0;
    uint32_t width              = 4000;
    uint32_t height             = 3000;
    uint32_t skyboxSize         = 1024;
    uint32_t brdfLUTSize        = 256;
    uint32_t shadowMapSize      = 4096;
    uint32_t highlightMapSize   = 1024;
    uint32_t bloomMapSize       = 1024; // size of bloom map using dual kawase blur algorithm
    uint32_t bloomMipLevels     = 4;    // number of mip levels for bloom map
    uint32_t lensflareMapSize   = 512;  // size of lensflare map using gaussian blur algorithm
    uint32_t lensflareBlurTimes = 2;    // number of gaussian blur times for lensflare map
};

} // namespace tinyglrenderer

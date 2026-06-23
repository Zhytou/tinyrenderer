#include <cstdint>

namespace tinyglrenderer {

struct DisplayInfo {
    float fps         = 0;
    float deltaTime   = 0;
    uint32_t drawCall = 0;
};

} // namespace tinyglrenderer
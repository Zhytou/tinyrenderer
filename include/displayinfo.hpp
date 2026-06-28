#include <cstdint>

namespace tinyglrenderer {

struct DisplayInfo {
    float fps       = 0;
    float deltaTime = 0;
    size_t drawCall = 0;
};

} // namespace tinyglrenderer
#include "utils.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace tinyglrenderer {

std::ostream& operator<<(std::ostream& stream, const glm::vec3& vec) {
    stream << vec.x << ' ' << vec.y << ' ' << vec.z;
    return stream;
}

std::ostream& operator<<(std::ostream& stream, const glm::vec4& vec) {
    stream << vec.x << ' ' << vec.y << ' ' << vec.z << ' ' << vec.w;
    return stream;
}

std::ostream& operator<<(std::ostream& stream, const glm::mat4& mat) {
    stream << mat[0] << '\n'
           << mat[1] << '\n'
           << mat[2] << '\n'
           << mat[3] << '\n';

    return stream;
}

void glDebug(const char* msg) {
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cout << msg << " openGL error" << ": 0x" << std::hex << err << std::dec << std::endl;
    }
}

}  // namespace tinyglrenderer

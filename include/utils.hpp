#pragma once

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <iostream>
#include <string>
#include <vector>

namespace tinyrenderer {
std::ostream& operator<<(std::ostream& stream, const glm::vec3& vec);
std::ostream& operator<<(std::ostream& stream, const glm::vec4& vec);
std::ostream& operator<<(std::ostream& stream, const glm::mat4& mat);

void glDebug(const char* msg);
}  // namespace tinyrenderer

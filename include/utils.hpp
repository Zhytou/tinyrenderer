#pragma once

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <iostream>
#include <string>
#include <vector>

namespace tinyglrenderer {

std::ostream& operator<<(std::ostream& stream, const glm::vec3& vec);

std::ostream& operator<<(std::ostream& stream, const glm::vec4& vec);

std::ostream& operator<<(std::ostream& stream, const glm::mat4& mat);

void glDebug(const char* msg);

const char* glMacro2Str(GLenum value);

}  // namespace tinyglrenderer

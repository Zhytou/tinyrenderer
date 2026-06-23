#pragma once

#include "bindablebuffer.hpp"

namespace tinyglrenderer {

/**
 * @brief Shared GPU storage interface block mapped to std430 layout structures.
 * @note Standardizes large-scale dynamic data streaming (e.g., LightBuffer, StructuredMesh)
 * across distinct pipelines, enabling unbounded array access without uniform storage limits.
 */
class ShaderStorageBuffer : public BindableBuffer {
   public:
    ShaderStorageBuffer(GLsizeiptr size, const void* data = nullptr) : BindableBuffer(GL_SHADER_STORAGE_BUFFER, size, data) {}
    ~ShaderStorageBuffer() {}
};

};  // namespace tinyglrenderer

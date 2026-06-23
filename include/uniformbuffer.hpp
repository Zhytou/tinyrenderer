#pragma once

#include "bindablebuffer.hpp"

namespace tinyglrenderer {

/**
 * @brief Shared GPU data interface block mapped to std140 layout structures.
 * @note Standardizes global context streaming (e.g., CameraBlock, LightBlock)
 * across distinct shaders without individual layout tracking overhead.
 */
class UniformBuffer : public BindableBuffer {
   public:
    UniformBuffer(GLsizeiptr size, const void* data = nullptr) : BindableBuffer(GL_UNIFORM_BUFFER, size, data) {}
    ~UniformBuffer() = default;
};

}  // namespace tinyglrenderer
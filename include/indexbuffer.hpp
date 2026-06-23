#pragma once

#include "graphicbuffer.hpp"

namespace tinyglrenderer {

/**
 * @brief Linear topology index stream allocator (Element/Index Buffer Object).
 * @details Stores unsigned primitives (indices) fed directly into the GPU's hardware
 * Primitive Assembly and Vertex Cache units. Bypasses general Vertex Shader inputs.
 * Managed asynchronously via `glVertexArrayElementBuffer` to dictate vertex assembly
 * sequences without mutating global state machines.
 */
class IndexBuffer : public GraphicBuffer {
   public:
    IndexBuffer(GLsizeiptr size, const void* data = nullptr) : GraphicBuffer(GL_ELEMENT_ARRAY_BUFFER, size, data) {}
    ~IndexBuffer() = default;
};

}  // namespace tinyglrenderer
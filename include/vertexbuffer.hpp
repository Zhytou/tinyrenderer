#pragma once

#include "graphicbuffer.hpp"

namespace tinyglrenderer {

/**
 * @brief Passive vertex attribute data stream container (Vertex Buffer Object).
 * @details Inherits the immutable raw memory base to act strictly as a data payload.
 * It does NOT hold any pipeline layout or binding state; rather, it is designed to be
 * hot-swapped into a VertexLayout (VAO) binding slot via non-binding DSA commands
 * (`glVertexArrayVertexBuffer`), minimizing heavy pipeline re-validations.
 * @note Typically used to store interleaved static mesh components (e.g., Position, Normal, UV).
 */
class VertexBuffer : public GraphicBuffer {
   public:
    VertexBuffer(GLsizeiptr size, const void* data = nullptr) : GraphicBuffer(GL_ARRAY_BUFFER, size, data) {}
    ~VertexBuffer() = default;
};

}  // namespace tinyglrenderer

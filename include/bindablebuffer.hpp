#pragma once

#include "graphicbuffer.hpp"

namespace tinyrenderer {

class BindableBuffer : public GraphicBuffer {
   public:
    BindableBuffer(GLenum type, GLsizeiptr size, const void* data = nullptr) : GraphicBuffer(type, size, data) {}
    ~BindableBuffer() = default;

    // Bind the whole uniform buffer to shader binding point.
    void bind(GLuint slot) const {
        glBindBufferBase(m_type, slot, m_id);
    }

    // Bind the subrange of uniform buffer to shader binding point.
    // @para
    void bind(GLuint slot, GLintptr offset, GLsizeiptr size) {
        glBindBufferRange(m_type, slot, m_id, offset, size);
    }
};

}  // namespace tinyrenderer

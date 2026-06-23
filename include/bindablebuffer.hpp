#pragma once

#include "graphicbuffer.hpp"

namespace tinyglrenderer {

class BindableBuffer : public GraphicBuffer {
   public:
    BindableBuffer(GLenum target, GLsizeiptr size, const void* data = nullptr) : GraphicBuffer(target, size, data) {}
    ~BindableBuffer() = default;

    // Bind the whole uniform buffer to shader binding point.
    void bind(GLuint slot) const {
        glBindBufferBase(m_target, slot, m_id);
    }

    // Bind the subrange of uniform buffer to shader binding point.
    // @para
    void bind(GLuint slot, GLintptr offset, GLsizeiptr size) {
        glBindBufferRange(m_target, slot, m_id, offset, size);
    }
};

}  // namespace tinyglrenderer

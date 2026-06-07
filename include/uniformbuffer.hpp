#include <glad/glad.h>

#include <iostream>

namespace tinyrenderer {

/**
 * @brief Shared GPU data interface block mapped to std140 layout structures.
 * @note Standardizes global context streaming (e.g., CameraBlock, LightBlock)
 * across distinct shaders without individual layout tracking overhead.
 */
class UniformBuffer : public GraphicBuffer {
   public:
    UniformBuffer(GLsizeiptr size, const void* data = nullptr) : GraphicBuffer(GL_UNIFORM_BUFFER, size, data) {}
    ~UniformBuffer() = default;

    // Bind the whole uniform buffer to shader binding point.
    void bind(GLuint slot) const {
        glBindBufferBase(m_type, slot, m_id);
    }

    // Bind the subrange of uniform buffer to shader binding point.
    void bind(GLuint slot, GLintptr offset, GLsizeiptr size) {
        glBindBufferRange(m_type, slot, m_id, offset, size);
    }
};

}  // namespace tinyrenderer
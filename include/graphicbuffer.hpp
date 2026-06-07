#pragma once

#include <glad/glad.h>

namespace tinyrenderer {
class GraphicBuffer {
   public:
    GraphicBuffer(GLenum type, GLsizeiptr size, const void* data = nullptr);
    ~GraphicBuffer();

    GraphicBuffer(const GraphicBuffer&)            = delete;
    GraphicBuffer& operator=(const GraphicBuffer&) = delete;

    GLuint getId() const { return m_id; }
    GLenum getType() const { return m_type; }

    void upload(GLintptr offset, GLsizeiptr length, const void* data);

   protected:
    GLuint m_id       = 0;
    GLenum m_type     = GL_ARRAY_BUFFER;  // could be vbo/ebo/ubo ...
    GLsizeiptr m_size = 0;
};

}  // namespace tinyrenderer
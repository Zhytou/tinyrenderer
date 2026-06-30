#pragma once

#include <glad/glad.h>

namespace tinyglrenderer {
class GraphicBuffer {
   public:
    GraphicBuffer(GLenum target, GLsizeiptr size, const void* data = nullptr);
    ~GraphicBuffer();

    GraphicBuffer(const GraphicBuffer&)            = delete;
    GraphicBuffer& operator=(const GraphicBuffer&) = delete;

    GLuint getID() const { return m_id; }
    GLenum getTarget() const { return m_target; }
    GLsizeiptr getSize() const { return m_size; }

    // Upload data to graphic buffer.
    // @param offset The offset of data to upload.
    // @param length The length of data to upload.
    // @param data The data to upload.
    void upload(GLintptr offset, GLsizeiptr length, const void* data);

    // Clear the sub range of graphic buffer to all zeros.
    // @param offset The offset of the sub range to clear.
    // @param length The length of the sub range to clear.
    void clear(GLintptr offset, GLsizeiptr length);

   protected:
    GLuint m_id       = 0;
    GLenum m_target   = GL_ARRAY_BUFFER;  // could be vbo/ebo/ubo ...
    GLsizeiptr m_size = 0;
};

}  // namespace tinyglrenderer
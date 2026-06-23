#pragma once

#include <glad/glad.h>

#include <memory>
#include <unordered_map>
#include <vector>

#include "indexbuffer.hpp"
#include "vertexbuffer.hpp"

namespace tinyglrenderer {

struct VertexAttribute {
    GLuint location      = 0;         // attribute index (e.g., layout location = 0)
    GLuint size          = 0;         // size of attribute(e.g., vec3 is 3)
    GLenum type          = GL_FLOAT;  // data type (e.g., GL_FLOAT)
    GLuint offset        = 0;         // offset of attribute in bytes (e.g., {vec3 pos; vec3 normal} 0 for position, 12 for normal)
    GLboolean normalized = GL_FALSE;  // whether to normalize the attribute (e.g., {vec3 pos; vec3 normal} GL_FALSE for position, GL_TRUE for normal)
    GLuint stride        = 0;         // stride of data in bytes (e.g., {vec3 pos; vec3 normal} 24 for each vertex)
    GLuint slot          = 0;         // vertex buffer object binding slot
};

/**
 * @brief Vertex Layout (VAO)
 *
 * Multiple VBOs, each attribute comes from its own buffer.
 *
 * ┌─────────────────────────────────────────────────────────────────────────────┐
 * │                              VertexLayout (VAO)                              │
 * ├─────────────────────────────────────────────────────────────────────────────┤
 * │                                                                             │
 * │  ┌─────────────────────────────────────────────────────────────────────┐   │
 * │  │                         Binding Slots                                │   │
 * │  │  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐               │   │
 * │  │  │  Slot 0      │  │  Slot 1      │  │  Slot 2      │               │   │
 * │  │  │ ┌──────────┐ │  │ ┌──────────┐ │  │ ┌──────────┐ │               │   │
 * │  │  │ │ VBO (Pos)│ │  │ │ VBO (Norm)│ │  │ │ VBO (UV) │ │               │   │
 * │  │  │ │ offset:0 │ │  │ │ offset:0 │ │  │ │ offset:0 │ │               │   │
 * │  │  │ │ stride:12│ │  │ │ stride:12│ │  │ │ stride:8 │ │               │   │
 * │  │  │ └──────────┘ │  │ └──────────┘ │  │ └──────────┘ │               │   │
 * │  │  └──────┬───────┘  └──────┬───────┘  └──────┬───────┘               │   │
 * │  └─────────┼─────────────────┼─────────────────┼───────────────────────┘   │
 * │            │                 │                 │                           │
 * │  ┌─────────▼─────────────────▼─────────────────▼───────────────────────┐   │
 * │  │                      Vertex Attributes                               │   │
 * │  │  location 0 ──── bindingIndex 0 ──── format (3 x float)            │   │
 * │  │  location 1 ──── bindingIndex 1 ──── format (3 x float)            │   │
 * │  │  location 2 ──── bindingIndex 2 ──── format (2 x float)            │   │
 * │  └─────────────────────────────────────────────────────────────────────┘   │
 * │                                                                             │
 * │  ┌─────────────────────────────────────────────────────────────────────┐   │
 * │  │                        Index Buffer (EBO)                           │   │
 * │  │  glVertexArrayElementBuffer(vao, ebo)                               │   │
 * │  └─────────────────────────────────────────────────────────────────────┘   │
 * └─────────────────────────────────────────────────────────────────────────────┘
 *
 * All attributes are interleaved in one VBO, using different relativeOffsets.
 *
 * ┌─────────────────────────────────────────────────────────────────────────────┐
 * │                          VertexLayout (VAO) - Interleaved                   │
 * ├─────────────────────────────────────────────────────────────────────────────┤
 * │                                                                             │
 * │  ┌─────────────────────────────────────────────────────────────────────┐   │
 * │  │                        Binding Slots (Single)                       │   │
 * │  │  ┌──────────────────────────────────────────────────────────────┐   │   │
 * │  │  │                         Slot 0                               │   │   │
 * │  │  │  ┌────────────────────────────────────────────────────────┐  │   │   │
 * │  │  │  │              VBO (Interleaved Data)                    │  │   │   │
 * │  │  │  │  ┌──────────┬──────────┬──────────┬──────────┐        │  │   │   │
 * │  │  │  │  │ Position │  Normal  │ TexCoord │ Position │ ...    │  │   │   │
 * │  │  │  │  │ (12 bytes)│(12 bytes)│ (8 bytes)│ (12 bytes)│        │  │   │   │
 * │  │  │  │  └──────────┴──────────┴──────────┴──────────┘        │  │   │   │
 * │  │  │  │                    Vertex 0            Vertex 1        │  │   │   │
 * │  │  │  │                                                         │  │   │   │
 * │  │  │  │  offset: 0      stride: 32 bytes                       │  │   │   │
 * │  │  │  └────────────────────────────────────────────────────────┘  │   │   │
 * │  │  └───────────────────────────┬──────────────────────────────────┘   │   │
 * │  └──────────────────────────────┼──────────────────────────────────────┘   │
 * │                                 │                                         │
 * │  ┌──────────────────────────────▼──────────────────────────────────────┐   │
 * │  │                      Vertex Attributes                               │   │
 * │  │                                                                      │   │
 * │  │  location 0 (Position)                                              │   │
 * │  │    └── bindingIndex 0 ──── relativeOffset 0    ──── format 3xfloat  │   │
 * │  │                                                                      │   │
 * │  │  location 1 (Normal)                                                │   │
 * │  │    └── bindingIndex 0 ──── relativeOffset 12   ──── format 3xfloat  │   │
 * │  │                                                                      │   │
 * │  │  location 2 (TexCoord)                                              │   │
 * │  │    └── bindingIndex 0 ──── relativeOffset 24   ──── format 2xfloat  │   │
 * │  │                                                                      │   │
 * │  └─────────────────────────────────────────────────────────────────────┘   │
 * │                                                                             │
 * │  ┌─────────────────────────────────────────────────────────────────────┐   │
 * │  │                        Index Buffer (EBO)                           │   │
 * │  │  glVertexArrayElementBuffer(vao, ebo)                               │   │
 * │  └─────────────────────────────────────────────────────────────────────┘   │
 * └─────────────────────────────────────────────────────────────────────────────┘
 */
class VertexLayout {
   public:
    VertexLayout();
    ~VertexLayout();

    // Set the input attributes of vertex layout(vao).
    // @param attributes The input attributes of vertex layout(vao)
    void initialize(const std::vector<VertexAttribute>& attributes);

    // Attach vertex buffer(vbo) to current vertex layout(vao)
    // @param slot The binding point of vertex buffer(vbo)
    // @param buffer The vertex buffer(vbo)
    // @param offset The offset of vertex buffer data(vbo)
    // @param stride The stride of vertex buffer data(vbo)
    // @return True if attachment is successful, False otherwise.
    bool attach(GLuint slot, const std::unique_ptr<VertexBuffer>& buffer, GLintptr offset, GLsizei stride) const;
    // Attach index buffer(ebo) to current vertex layout(vao)
    // @param buffer The index buffer(ebo)
    // @return True if attachment is successful, False otherwise.
    bool attach(const std::unique_ptr<IndexBuffer>& buffer) const;

    // Bind current vertex layout(vao)
    void bind() const;

    GLuint getId() const { return m_id; }

   private:
    GLuint m_id = 0;
};

}  // namespace tinyglrenderer
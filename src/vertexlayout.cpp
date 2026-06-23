#include "vertexlayout.hpp"

namespace tinyglrenderer {

VertexLayout::VertexLayout() {
    // Create vertex array object
    //
    // ┌──────────────────────────────────────────────────────────────────────────────────────────────┐
    // │                         glGenVertexArrays vs glCreateVertexArrays                            │
    // ├───────────────────────────────────────────────┬──────────────────────────────────────────────┤
    // │              glGenVertexArrays                │              glCreateVertexArrays            │
    // ├───────────────────────────────────────────────┼──────────────────────────────────────────────┤
    // │ • Generates UNINITIALIZED VAO ID              │ • Creates and INITIALIZES VAO object         │
    // │ • Requires separate glBindVertexArray call    │ • No bind needed (DSA style)                  │
    // │ • Must bind before configuring vertex attrs   │ • Configure directly without binding         │
    // │ • Changes global VAO binding state            │ • Direct state access, no side effects       │
    // │ • Must call glBindVertexArray before upload   │ • Configure via glVertexArray... APIs        │
    // │ • Traditional API (OpenGL 3.0+)               │ • Modern API (OpenGL 4.5+ / ARB_dsa)          │
    // │ • More verbose (gen + bind + config)          │ • Concise (create + direct config)           │
    // │ • Error prone (forget to bind)                │ • Safer (no binding state to track)          │
    // └───────────────────────────────────────────────┴──────────────────────────────────────────────┘
    //
    // Key insight:  glGenVertexArrays     = "allocate ID only" (need bind to initialize)
    //               glCreateVertexArrays  = "allocate + initialize" (ready to configure directly)
    //
    // Example:
    //   Legacy: glGenVertexArrays(1, &vao); glBindVertexArray(vao); glVertexAttribPointer(...);
    //   Modern: glCreateVertexArrays(1, &vao); glVertexArrayAttribFormat(vao, ...);
    //
    glCreateVertexArrays(1, &m_id);
}

VertexLayout::~VertexLayout() {
    if (m_id) {
        glDeleteVertexArrays(1, &m_id);
    }
}

void VertexLayout::initialize(const std::vector<VertexAttribute>& attributes) {
    for (const auto& attr : attributes) {
        glEnableVertexArrayAttrib(m_id, attr.location);
        glVertexArrayAttribFormat(m_id, attr.location, attr.size, attr.type, attr.normalized, attr.offset);
        glVertexArrayAttribBinding(m_id, attr.location, attr.slot);
    }
}

bool VertexLayout::attach(GLuint slot, const std::unique_ptr<VertexBuffer>& buffer, GLintptr offset, GLsizei stride) const {
    if (buffer) {
        glVertexArrayVertexBuffer(m_id, slot, buffer->getId(), offset, stride);
        return true;
    }
    return false;
}

bool VertexLayout::attach(const std::unique_ptr<IndexBuffer>& buffer) const {
    if (buffer) {
        glVertexArrayElementBuffer(m_id, buffer->getId());
        return true;
    }
    return false;
}

void VertexLayout::bind() const {
    glBindVertexArray(m_id);
}

}  // namespace tinyglrenderer
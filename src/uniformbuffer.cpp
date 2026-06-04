#include "uniformbuffer.hpp"

namespace tinyrenderer {

UniformBuffer::UniformBuffer(size_t size) : m_size(size) {
    // Create uniform buffer handle
    //
    // ┌──────────────────────────────────────────────────────────────────────────────────────────────┐
    // │                          glGenBuffers vs glCreateBuffers                                     │
    // ├───────────────────────────────────────────────┬──────────────────────────────────────────────┤
    // │              glGenBuffers                     │              glCreateBuffers                 │
    // ├───────────────────────────────────────────────┼──────────────────────────────────────────────┤
    // │ • Generates UNINITIALIZED buffer IDs          │ • Creates and INITIALIZES buffer objects     │
    // │ • Requires separate glBindBuffer call         │ • No bind needed (DSA style)                  │
    // │ • Must specify target at bind time            │ • Target specified at creation               │
    // │ • Changes global buffer binding state         │ • Direct state access, no side effects       │
    // │ • Must call glBindBuffer before upload        │ • Upload directly via glNamedBuffer... APIs  │
    // │ • Traditional API (OpenGL 1.5+)               │ • Modern API (OpenGL 4.5+ / ARB_dsa)          │
    // │ • More verbose (gen + bind + upload)          │ • Concise (create + direct upload)           │
    // │ • Error prone (forget to bind)                │ • Safer (no binding state to track)          │
    // └───────────────────────────────────────────────┴──────────────────────────────────────────────┘
    //
    // Key insight:  glGenBuffers    = "allocate ID only" (need bind + target to use)
    //               glCreateBuffers = "allocate + initialize + target" (ready to use immediately)
    //
    // Example:
    //   Legacy: glGenBuffers(1, &vbo); glBindBuffer(GL_ARRAY_BUFFER, vbo); glBufferData(...);
    //   Modern: glCreateBuffers(1, &vbo); glNamedBufferData(vbo, size, data, GL_STATIC_DRAW);
    glCreateBuffers(1, &m_id);

    // Upload data to the uniform buffer object
    //
    // ┌──────────────────────────────────────────────────────────────────────────────────────────────┐
    // │                          glBufferData vs glNamedBufferData                                   │
    // ├───────────────────────────────────────────────┬──────────────────────────────────────────────┤
    // │              glBufferData                     │              glNamedBufferData               │
    // ├───────────────────────────────────────────────┼──────────────────────────────────────────────┤
    // │ • Requires currently BOUND buffer             │ • Directly specifies buffer ID               │
    // │ • Changes global binding state                │ • No binding required (DSA style)            │
    // │ • Must call glBindBuffer before upload        │ • Upload directly without bind               │
    // │ • Target parameter needed (GL_ARRAY_BUFFER)   │ • No target needed (inferred from object)    │
    // │ • Traditional API (OpenGL 1.5+)               │ • Modern API (OpenGL 4.5+ / ARB_dsa)         │
    // │ • Error prone (wrong buffer bound)            │ • Safer (explicit buffer ID)                 │
    // │ • Affects other operations using same target  │ • No side effects on global state            │
    // └───────────────────────────────────────────────┴──────────────────────────────────────────────┘
    //
    // Key insight:  glBufferData     = "bind then upload" (state-dependent)
    //               glNamedBufferData = "upload directly" (stateless, explicit)
    //
    // Example:
    //   Legacy: glBindBuffer(GL_ARRAY_BUFFER, vbo); glBufferData(GL_ARRAY_BUFFER, size, data, usage);
    //   Modern: glNamedBufferData(vbo, size, data, usage);  // No bind needed!
    glNamedBufferData(m_id, size, nullptr, GL_DYNAMIC_DRAW);
}
UniformBuffer::~UniformBuffer() {
    if (m_id != 0) {
        glDeleteBuffers(1, &m_id);
    }
}

void UniformBuffer::bind(uint32_t unit) const {
    glBindBufferBase(GL_UNIFORM_BUFFER, unit, m_id);
}

void UniformBuffer::upload(const void* data) {
    // Upload a subset of buffer data
    //
    // ┌──────────────────────────────────────────────────────────────────────────────────────────────┐
    // │                     glBufferSubData vs glNamedBufferSubData                                  │
    // ├───────────────────────────────────────────────┬──────────────────────────────────────────────┤
    // │              glBufferSubData                  │              glNamedBufferSubData            │
    // ├───────────────────────────────────────────────┼──────────────────────────────────────────────┤
    // │ • Requires currently BOUND buffer             │ • Directly specifies buffer ID               │
    // │ • Changes global binding state                │ • No binding required (DSA style)            │
    // │ • Must call glBindBuffer before update        │ • Update directly without bind               │
    // │ • Target parameter needed (GL_ARRAY_BUFFER)   │ • No target needed (inferred from object)    │
    // │ • Error prone (wrong buffer bound)            │ • Safer (explicit buffer ID)                 │
    // │ • Traditional API (OpenGL 1.5+)               │ • Modern API (OpenGL 4.5+ / ARB_dsa)         │
    // │ • Affects other operations using same target  │ • No side effects on global state            │
    // └───────────────────────────────────────────────┴──────────────────────────────────────────────┘
    //
    // Key insight:  glBufferSubData     = "bind then update" (state-dependent)
    //               glNamedBufferSubData = "update directly" (stateless, explicit)
    //
    // Example:
    //   Legacy: glBindBuffer(GL_ARRAY_BUFFER, vbo); glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
    //   Modern: glNamedBufferSubData(vbo, offset, size, data);  // No bind needed!
    //
    // Note: Buffer must already have storage allocated (via glBufferData or glNamedBufferData)
    //
    glNamedBufferSubData(m_id, 0, m_size, data);
}

}  // namespace tinyrenderer
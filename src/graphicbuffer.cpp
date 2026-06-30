#include "graphicbuffer.hpp"

#include <stdexcept>

namespace tinyglrenderer {
GraphicBuffer::GraphicBuffer(GLenum target, GLsizeiptr size, const void* data) : m_target(target), m_size(size) {
    if (size <= 0) {
        // by the way, data can be nullptr when construting a new buffer, which means only allocating memory without initializing
        throw std::invalid_argument("GraphicBuffer::GraphicBuffer: size must be greater than 0");
    }

    // Create graphic buffer handle
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
    //   Legacy: glGenBuffers(1, &id); glBindBuffer(GL_ARRAY_BUFFER, id); glBufferData(...);
    //   Modern: glCreateBuffers(1, &id); glNamedBufferData(id, size, data, GL_STATIC_DRAW);
    glCreateBuffers(1, &m_id);

    // Upload data to the graphic buffer object
    // ┌────────────────────────────────────────────────────────────────────────────────────────────────────────────┐
    // │                              glBufferData vs glNamedBufferData vs glNamedBufferStorage                       │
    // ├─────────────────────────────────────┬─────────────────────────────────┬──────────────────────────────────────┤
    // │           glBufferData              │         glNamedBufferData       │        glNamedBufferStorage           │
    // ├─────────────────────────────────────┼─────────────────────────────────┼──────────────────────────────────────┤
    // │ • Requires BINDING buffer           │ • Directly specifies buffer ID  │ • Directly specifies buffer ID        │
    // │ • Changes global state              │ • No binding (DSA style)        │ • No binding (DSA style)              │
    // │ • Can RE-ALLOCATE anytime           │ • Can RE-ALLOCATE anytime       │ • IMMUTABLE size (once allocated)     │
    // │ • Mutable storage                   │ • Mutable storage               │ • IMMUTABLE storage                   │
    // │ • Traditional API (OpenGL 1.5+)     │ • Modern API (OpenGL 4.5+)      │ • Modern API (OpenGL 4.5+)            │
    // │ • Slower (realloc possible)         │ • Slower (realloc possible)     │ • FASTER (driver can optimize)        │
    // │ • Usage: GL_STATIC_DRAW, etc.       │ • Usage: GL_STATIC_DRAW, etc.   │ • Usage: GL_MAP_*_BIT flags           │
    // └─────────────────────────────────────┴─────────────────────────────────┴──────────────────────────────────────┘
    //
    // Key insight:  glBufferData        = "bind then allocate + upload" (mutable, stateful)
    //               glNamedBufferData   = "allocate + upload directly" (mutable, stateless)
    //               glNamedBufferStorage = "allocate IMMUTABLE storage" (immutable, faster, preferred)
    //
    // Example:
    //   Legacy:   glBindBuffer(GL_ARRAY_BUFFER, id); glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    //   Modern:   glNamedBufferData(id, size, data, GL_STATIC_DRAW);
    //   Immutable:glNamedBufferStorage(id, size, data, GL_DYNAMIC_STORAGE_BIT);  // Size cannot change!
    glNamedBufferStorage(m_id, size, data, GL_DYNAMIC_STORAGE_BIT);
}

GraphicBuffer::~GraphicBuffer() {
    if (m_id != 0) {
        glDeleteBuffers(1, &m_id);
    }
}

void GraphicBuffer::upload(GLintptr offset, GLsizeiptr length, const void* data) {
    if (offset < 0 || offset + length > m_size) {
        throw std::runtime_error("GraphicBuffer::upload: offset or length out of range!");
    }
    if (data == nullptr) {
        throw std::invalid_argument("GraphicBuffer::upload: data cannot be nullptr");
    }

    // Upload data to the graphic buffer object at the specified offset and length
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
    //   Legacy: glBindBuffer(GL_ARRAY_BUFFER, id); glBufferSubData(GL_ARRAY_BUFFER, offset, length, data);
    //   Modern: glNamedBufferSubData(id, offset, length, data);  // No bind needed!
    //
    // Note: Buffer must already have storage allocated (via glBufferData or glNamedBufferData)
    glNamedBufferSubData(m_id, offset, length, data);
}

void GraphicBuffer::clear(GLintptr offset, GLsizeiptr length) {
    if (offset < 0 || offset + length > m_size) {
        throw std::runtime_error("GraphicBuffer::clear: offset or length out of range!");
    }

    // Clear the sub range of graphic buffer to all zeros
    GLuint zero = 0;
    glClearNamedBufferSubData(
        m_id, GL_R32UI,    // treat the buffer as a series of 32-bit unsigned integers
        offset, length, 
        GL_RED_INTEGER,    // indicate that zero is a single-channel integer value
        GL_UNSIGNED_INT,   // indicate that zero is an unsigned int
        &zero
    );
}

} // namespace tinyglrenderer
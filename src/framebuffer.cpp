#include "framebuffer.hpp"

#include <iostream>

#include "utils.hpp"

namespace tinyrenderer {

FrameBuffer::FrameBuffer(bool screen, uint32_t w, uint32_t h) : m_width(w), m_height(h) {
    if (!screen) {
        // Create framebuffer object
        //
        // ┌──────────────────────────────────────────────────────────────────────────────────────────────┐
        // │                        glGenFramebuffers vs glCreateFramebuffers                              │
        // ├───────────────────────────────────────────────┬──────────────────────────────────────────────┤
        // │              glGenFramebuffers                │              glCreateFramebuffers             │
        // ├───────────────────────────────────────────────┼──────────────────────────────────────────────┤
        // │ • Generates UNINITIALIZED IDs                 │ • Creates and INITIALIZES objects            │
        // │ • Requires separate glBindFramebuffer call    │ • No bind needed (DSA style)                  │
        // │ • Changes global binding state                │ • Direct state access, no side effects       │
        // │ • Must call glBindFramebuffer before config   │ • Configure directly via glNamed... APIs     │
        // │ • Traditional API (OpenGL 3.0+)               │ • Modern API (OpenGL 4.5+ / ARB_dsa)          │
        // │ • More verbose (gen + bind + config)          │ • Concise (create + direct config)           │
        // │ • Error prone (forget to bind)                │ • Safer (no binding state to track)          │
        // └───────────────────────────────────────────────┴──────────────────────────────────────────────┘
        //
        // Key insight:  glGenFramebuffers  = "allocate ID only" (need bind to use)
        //               glCreateFramebuffers = "allocate + initialize" (ready to use immediately)
        glCreateFramebuffers(1, &m_id);
    }
}

FrameBuffer::~FrameBuffer() {
    if (m_id) {
        glDeleteFramebuffers(1, &m_id);
    }
    m_attachments.clear();
}

FrameBuffer::FrameBuffer(FrameBuffer&& other) {
    if (m_id) {
        glDeleteFramebuffers(1, &m_id);
        m_attachments.clear();
    }

    m_id          = other.m_id;
    m_width       = other.m_width;
    m_height      = other.m_height;
    m_attachments = std::move(other.m_attachments);

    other.m_id     = 0;
    other.m_width  = 0;
    other.m_height = 0;
    other.m_attachments.clear();
}

FrameBuffer& FrameBuffer::operator=(FrameBuffer&& other) {
    if (m_id) {
        glDeleteFramebuffers(1, &m_id);
        m_attachments.clear();
    }

    m_id          = other.m_id;
    m_width       = other.m_width;
    m_height      = other.m_height;
    m_attachments = std::move(other.m_attachments);

    other.m_id     = 0;
    other.m_width  = 0;
    other.m_height = 0;
    other.m_attachments.clear();
    return *this;
};

void FrameBuffer::bind() const {
    glViewport(0, 0, m_width, m_height);
    glBindFramebuffer(GL_FRAMEBUFFER, m_id);
}

void FrameBuffer::attach(GLenum slot, const std::shared_ptr<Texture>& texture) {
    if (m_id == 0) {
        throw std::runtime_error("FrameBuffer::attach: framebuffer not created");
    }
    if (m_attachments.count(slot)) {
        throw std::runtime_error("FrameBuffer::attach: attachment already attached");
    }
    m_attachments[slot] = texture;

    // Attach a 2D texture level to a framebuffer
    //
    // ┌──────────────────────────────────────────────────────────────────────────────────────────────┐
    // │                     glFramebufferTexture2D vs glNamedFramebufferTexture                      │
    // ├───────────────────────────────────────────────┬──────────────────────────────────────────────┤
    // │              glFramebufferTexture2D           │              glNamedFramebufferTexture       │
    // ├───────────────────────────────────────────────┼──────────────────────────────────────────────┤
    // │ • Requires currently BOUND framebuffer        │ • Directly specifies framebuffer ID          │
    // │ • Changes global framebuffer binding state    │ • No binding required (DSA style)            │
    // │ • Must specify texture target                 │ • No texture target needed                   │
    // │   (GL_TEXTURE_2D, GL_TEXTURE_CUBE_MAP_POS_X)  │   (inferred from texture object)            │
    // │ • Need separate glBindFramebuffer call        │ • One call, direct state access              │
    // │ • Traditional API (OpenGL 3.0+)               │ • Modern API (OpenGL 4.5+ / ARB_dsa)         │
    // │ • Error prone (forget to bind)                │ • Safer (explicit framebuffer ID)            │
    // └───────────────────────────────────────────────┴──────────────────────────────────────────────┘
    //
    // Key insight:  glFramebufferTexture2D   = "bind then attach" (state-dependent)
    //               glNamedFramebufferTexture = "attach directly" (stateless, explicit)
    glNamedFramebufferTexture(m_id, slot, texture->getId(), 0);
}

void FrameBuffer::clear(GLenum slot, const glm::vec4& color, float depth, int stencil) {
    if (m_id == 0) {
        glClearColor(color.x, color.y, color.z, color.w);
        glClearDepth(depth);
        glClearStencil(stencil);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        return;
    }

    if (m_attachments.count(slot) == 0) {
        return;
    }
    if (slot == GL_DEPTH_STENCIL_ATTACHMENT) {
        glClearNamedFramebufferfv(m_id, GL_DEPTH, 0, &depth);
    } else if (slot == GL_STENCIL_ATTACHMENT) {
        glClearNamedFramebufferiv(m_id, GL_STENCIL, 0, &stencil);
    } else {
        // Clear color attachment
        //
        // ┌──────────────────────────────────────────────────────────────────────────────────────────────┐
        // │                     glClearBufferfv vs glClearNamedFramebufferfv                            │
        // ├───────────────────────────────────────────────┬──────────────────────────────────────────────┤
        // │              glClearBufferfv                  │              glClearNamedFramebufferfv       │
        // ├───────────────────────────────────────────────┼──────────────────────────────────────────────┤
        // │ • Clears currently BOUND framebuffer          │ • Directly specifies framebuffer ID          │
        // │ • Requires glBindFramebuffer first            │ • No bind required (DSA style)                │
        // │ • Traditional API (OpenGL 3.0+)               │ • Modern API (OpenGL 4.5+ / ARB_dsa)         │
        // │ • State-dependent, error prone                │ • Explicit, stateless, safer                 │
        // └───────────────────────────────────────────────┴──────────────────────────────────────────────┘
        //
        // Key insight:  glClearBufferfv        = "bind then clear" (state-dependent)
        //               glClearNamedFramebufferfv = "clear directly" (stateless, explicit)
        //
        // Suffix meanings: fv = float vector → clears COLOR (RGBA) or DEPTH (single float or float vector)
        //                  fi = float + int  → clears DEPTH_STENCIL (depth + stencil together)
        //                  iv = int vector → clears STENCIL (single int or int vector)
        glClearNamedFramebufferfv(m_id, GL_COLOR, slot - GL_COLOR_ATTACHMENT0, &color.x);
    }
}

}  // namespace tinyrenderer

#pragma once
#include <glad/glad.h>

#include <glm/glm.hpp>
#include <unordered_map>

#include "texture.hpp"

namespace tinyrenderer {

/**
 * @brief Explicit physical render target allocation block.
 * @note Serves as the actual VRAM instance containing color, depth, or stencil surfaces.
 * Managed as a concrete asset bucket mapping directly to standard or off-screen render layouts.
 */
class FrameBuffer {
   public:
    FrameBuffer(bool screen, uint32_t w, uint32_t h);
    ~FrameBuffer();
    FrameBuffer(const FrameBuffer&)            = delete;
    FrameBuffer& operator=(const FrameBuffer&) = delete;
    FrameBuffer(FrameBuffer&& other);
    FrameBuffer& operator=(FrameBuffer&& other);

    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    void setWidth(uint32_t w) { m_width = w; }
    void setHeight(uint32_t h) { m_height = h; }

    //
    void bind() const;
    //
    void attach(GLenum slot, const std::shared_ptr<Texture>& texture);
    //
    void clear(GLenum slot, const glm::vec4& color = {0.0f, 0.0f, 0.0f, 1.0f}, float depth = 1.f, int stencil = 0);

   private:
    GLuint m_id       = 0;
    uint32_t m_width  = 0;
    uint32_t m_height = 0;
    std::unordered_map<GLenum, std::shared_ptr<Texture>> m_attachments;
};

}  // namespace tinyrenderer

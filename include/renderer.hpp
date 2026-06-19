#pragma once
#include <glad/glad.h>

#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <unordered_map>

#include "bindablebuffer.hpp"
#include "framebuffer.hpp"
#include "pipelinestate.hpp"
#include "renderitem.hpp"
#include "renderpass.hpp"
#include "sampler.hpp"
#include "scene.hpp"
#include "shader.hpp"
#include "vertexbuffer.hpp"
#include "vertexlayout.hpp"

namespace tinyrenderer {

/**
 * @brief Renderer class
 *
 * @details Renderer class is responsible for rendering the scene.
 * It initializes OpenGL context, loads assets, and renders the scene.
 */
class Renderer {
   public:
    Renderer(uint32_t width, uint32_t height) : m_width(width), m_height(height) {}
    ~Renderer() { shutdown(); }

    void setup();
    void shutdown();
    // Convert .hdr skybox to cubemap and bake the IBL environment map
    void prepare(const Scene& scene);
    // Update ubo/ssbo and bake shadow map
    void update(const Scene& scene);
    // Render scene
    void render(const Scene& scene);

    bool isShadowMapEnabled() const { return m_enableShadow; }
    bool isIBLEnabled() const { return m_enableIBL; }
    uint32_t getDrawCall() const { return m_drawCall; }
    void setShadowMap(bool enable) { m_enableShadow = enable; }
    void setIBL(bool enable) { m_enableIBL = enable; }
    void setShadowMapSize(uint32_t width, uint32_t height) {
        m_shadowMapWidth  = width;
        m_shadowMapHeight = height;
    }
    void setSkyboxSize(uint32_t size) { m_skyboxSize = size; }

   private:
    // draw mesh
    void draw(const RenderItem& item, const std::vector<std::string>& textures);
    // draw quad or skybox
    void draw(const std::shared_ptr<VertexLayout>& layout, const std::vector<std::string>& textures, GLsizei count);

    /// fixed-function states
    // input and output attachments
    std::unordered_map<std::string, RenderPass> m_passes;
    // fixed-function states
    std::unordered_map<std::string, PipelineState> m_states;

    /// assets and resources
    std::unordered_map<uint32_t, std::shared_ptr<Sampler>> m_samplers;
    std::unordered_map<std::string, std::shared_ptr<Shader>> m_shaders;
    std::unordered_map<std::string, std::shared_ptr<Texture>> m_textures;
    std::unordered_map<std::string, std::shared_ptr<FrameBuffer>> m_frames;
    std::unordered_map<std::string, std::shared_ptr<BindableBuffer>> m_buffers;

    // name mappings
    std::unordered_map<std::string, std::string> m_pass2FrameNames;
    std::unordered_map<std::string, uint32_t> m_texture2SlotIndexs;

    bool m_enableIBL = true, m_enableShadow = false;
    uint32_t m_width = 800, m_height = 600;
    uint32_t m_skyboxSize = 512, m_brdfLUTSize = 512;
    uint32_t m_shadowMapWidth = 4096, m_shadowMapHeight = 4096;
    uint32_t m_drawCall = 0;
};

}  // namespace tinyrenderer

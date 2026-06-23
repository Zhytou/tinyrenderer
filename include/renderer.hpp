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
    Renderer()  = default;
    ~Renderer() = default;

    void setup();
    void shutdown();
    // Convert .hdr skybox to cubemap and bake the IBL environment map
    void prepare(const Scene& scene);
    // Update ubo/ssbo and bake shadow map
    void update(const Scene& scene);
    // Render scene
    void render(const Scene& scene);

    void enableIBL(bool enable) { m_ibl = enable; }
    void enableShadow(bool enable) { m_shadow = enable; }
    void enableBloom(bool enable) { m_bloom = enable; }
    void enableSSAO(bool enable) { m_ssao = enable; }
    void enableTAA(bool enable) { m_taa = enable; }
    bool isIBLEnabled() const { return m_ibl; }
    bool isShadowEnabled() const { return m_shadow; }
    bool isBloomEnabled() const { return m_bloom; }
    bool isSSAOEnabled() const { return m_ssao; }
    bool isTAAEnabled() const { return m_taa; }

    uint32_t getDrawCall() const { return m_drawCall; }
    std::pair<uint32_t, uint32_t> getSize() const { return {m_width, m_height}; }
    uint32_t getShadowMapSize() const { return m_shadowMapSize; }
    uint32_t getSkyboxSize() const { return m_skyboxSize; }
    uint32_t getBRDFLUTSize() const { return m_brdfLUTSize; }
    void setSize(uint32_t width, uint32_t height) {
        m_width  = width;
        m_height = height;
    }
    void setShadowMapSize(uint32_t size) { m_shadowMapSize = size; }
    void setSkyboxSize(uint32_t size) { m_skyboxSize = size; }
    void setBRDFLUTSize(uint32_t size) { m_brdfLUTSize = size; }

   private:
    // draw mesh
    void draw(const RenderItem& item, const std::vector<std::string>& textures);
    // draw quad or skybox
    void draw(const std::shared_ptr<VertexLayout>& layout, const std::vector<std::string>& textures, GLsizei count);

    // input and output attachments
    std::unordered_map<std::string, RenderPass> m_passes;
    // fixed-function states
    std::unordered_map<std::string, PipelineState> m_states;

    // assets and resources
    std::unordered_map<std::string, std::shared_ptr<Sampler>> m_samplers;
    std::unordered_map<std::string, std::shared_ptr<Shader>> m_shaders;
    std::unordered_map<std::string, std::shared_ptr<Texture>> m_textures;
    std::unordered_map<std::string, std::shared_ptr<FrameBuffer>> m_frames;
    std::unordered_map<std::string, std::shared_ptr<BindableBuffer>> m_buffers;

    /// name mappings
    std::unordered_multimap<std::string, std::string> m_pass2FrameNames;
    std::unordered_map<std::string, uint32_t> m_texture2SlotIndexs;

    /// renderer settings
    bool m_deferred  = false;  // deferred rendering enabled or not
    bool m_ibl       = false;  // image based light enabled or not
    bool m_shadow    = false;  // shadow mapping enabled or not
    bool m_bloom     = true;   // bloom blur enabled or not
    bool m_lensflare = true;   // lensflare enabled or not
    bool m_dirtmask  = true;   // dirtmask enabled or not
    bool m_ssao      = false;  // screen space ambient occlusion enabled or not
    bool m_taa       = false;  // temporal anti aliasing enabled or not

    uint32_t m_width              = 4000;
    uint32_t m_height             = 3000;
    uint32_t m_skyboxSize         = 1024;
    uint32_t m_brdfLUTSize        = 256;
    uint32_t m_shadowMapSize      = 4096;
    uint32_t m_highlightMapSize   = 1024;
    uint32_t m_bloomMapSize       = 1024;  // size of bloom map using dual kawase blur algorithm
    uint32_t m_bloomMipLevels     = 4;     // number of mip levels for bloom map
    uint32_t m_lensflareMapSize   = 512;   // size of lensflare map using gaussian blur algorithm
    uint32_t m_lensflareBlurTimes = 2;     // number of gaussian blur times for lensflare map
    uint32_t m_drawCall           = 0;
};

}  // namespace tinyrenderer

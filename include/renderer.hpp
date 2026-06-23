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
#include "rendersetting.hpp"

namespace tinyglrenderer {

/**
 * @brief Renderer class
 *
 * @details Renderer class is responsible for rendering the scene.
 * It initializes OpenGL context, loads assets, and renders the scene.
 */
class Renderer {
   public:
    Renderer()  = default;
    ~Renderer() = default;
    Renderer(const Renderer& renderer) = delete;
    Renderer& operator=(const Renderer& renderer) = delete;

    void setup();
    void shutdown();
    // Convert .hdr skybox to cubemap and bake the IBL environment map
    void prepare(const Scene& scene);
    // Update ubo/ssbo and bake shadow map
    void update(const Scene& scene);
    // Render scene
    void render(const Scene& scene);
    // Config renderer settings
    void config(const RenderSetting& setting) { m_setting = setting; }
    // Resize renderer window
    void resize(uint32_t width, uint32_t height) { m_setting.width = width; m_setting.height = height; }

    uint32_t getDrawCall() const { return m_drawCall; }
    RenderSetting getSetting() const { return m_setting; }

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
    RenderSetting m_setting;
    uint32_t m_drawCall = 0;
};

}  // namespace tinyglrenderer

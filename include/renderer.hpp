#pragma once
#include <glad/glad.h>

#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <unordered_map>

#include "framebuffer.hpp"
#include "pipelinestate.hpp"
#include "renderitem.hpp"
#include "renderpass.hpp"
#include "scene.hpp"
#include "shader.hpp"
#include "uniformbuffer.hpp"
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
    void prepare(const Scene& scene);
    void render(const Scene& scene);

   private:
    // draw mesh
    static void draw(const RenderItem& item);
    // draw quad or skybox
    static void draw(const std::shared_ptr<VertexLayout>& layout, const std::unique_ptr<VertexBuffer>& buffer, const std::vector<std::shared_ptr<Texture>>& textures);

    /// fixed-function states
    // input and output attachments
    std::unordered_map<std::string, RenderPass> m_passes;
    // fixed-function states
    std::unordered_map<std::string, PipelineState> m_states;

    /// assets and resources
    std::unordered_map<std::string, std::shared_ptr<Shader>> m_shaders;
    std::unordered_map<std::string, std::shared_ptr<Texture>> m_textures;
    std::unordered_map<std::string, std::shared_ptr<FrameBuffer>> m_frames;
    std::unordered_map<std::string, std::shared_ptr<UniformBuffer>> m_uniforms;

    uint32_t m_width, m_height;
};

}  // namespace tinyrenderer

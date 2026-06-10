#include "renderer.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

#include "camera.hpp"
#include "model.hpp"
#include "renderitem.hpp"
#include "staticresource.hpp"
#include "utils.hpp"

namespace tinyrenderer {

void Renderer::setup() {
    // 1. Initialize renderpasses, namely define the input and output attachments of each pipeline
    m_passes["shadow_mapping"] = RenderPass{
        .attachments = {
            AttachmentDesc{
                .name   = "shadow_map",
                .target = GL_DEPTH,
                .type   = GL_TEXTURE_2D,
                .format = GL_DEPTH_COMPONENT24,
                .slot   = GL_DEPTH_ATTACHMENT,
                .loadOp = LoadOp::LOAD_OP_CLEAR,
                .value  = {.depthStencil = {1.0f, 0}},
            },
        },
    };
    m_passes["deferred_geometry"] = RenderPass{
        .attachments = {
            AttachmentDesc{
                .name   = "albedo",
                .target = GL_COLOR,
                .type   = GL_TEXTURE_2D,
                .format = GL_RGBA8,
                .slot   = GL_COLOR_ATTACHMENT0,
                .loadOp = LoadOp::LOAD_OP_CLEAR,
                .value  = {.color = {0.0f, 0.0f, 0.0f, 1.0f}},
            },
            AttachmentDesc{
                .name   = "normal",
                .target = GL_COLOR,
                .type   = GL_TEXTURE_2D,
                .format = GL_RGBA8,
                .slot   = GL_COLOR_ATTACHMENT1,
                .loadOp = LoadOp::LOAD_OP_CLEAR,
                .value  = {.color = {0.0f, 0.0f, 0.0f, 1.0f}},
            },
            AttachmentDesc{
                .name   = "mrao",
                .target = GL_COLOR,
                .type   = GL_TEXTURE_2D,
                .format = GL_RGBA8,
                .slot   = GL_COLOR_ATTACHMENT2,
                .loadOp = LoadOp::LOAD_OP_CLEAR,
                .value  = {.color = {0.0f, 0.0f, 0.0f, 1.0f}},
            },
            AttachmentDesc{
                .name   = "depth",
                .target = GL_DEPTH,
                .type   = GL_TEXTURE_2D,
                .format = GL_DEPTH_COMPONENT24,
                .slot   = GL_DEPTH_ATTACHMENT,
                .loadOp = LoadOp::LOAD_OP_CLEAR,
                .value  = {.depthStencil = {1.0f, 0}},
            },
        },
    };
    m_passes["deferred_shading"] = RenderPass{
        .attachments = {
            AttachmentDesc{
                .name   = "default_color",
                .target = GL_COLOR,
                .slot   = GL_BACK,
                .loadOp = LoadOp::LOAD_OP_CLEAR,
                .value  = {.color = {0.0f, 0.0f, 0.0f, 1.0f}},
            },
            AttachmentDesc{
                .name   = "default_depth",
                .target = GL_DEPTH,
                .slot   = GL_DEPTH_ATTACHMENT,
                .loadOp = LoadOp::LOAD_OP_DONT_CARE
            },
        },
    };
    m_passes["forward_opaque"] = RenderPass{
        .attachments = {
            AttachmentDesc{
                .name   = "default_color",
                .target = GL_COLOR,
                .slot   = GL_BACK,
                .loadOp = LoadOp::LOAD_OP_CLEAR,
                .value  = {.color = {0.0f, 0.0f, 0.0f, 1.0f}},
            },
            AttachmentDesc{
                .name   = "default_depth",
                .target = GL_DEPTH,
                .slot   = GL_DEPTH_ATTACHMENT,
                .loadOp = LoadOp::LOAD_OP_CLEAR,
                .value  = {.depthStencil = {1.0f, 0}},
            },
        },
    };
    m_passes["skybox"] = RenderPass{
        .attachments = {
            AttachmentDesc{
                .name   = "default_color",
                .target = GL_COLOR,
                .slot   = GL_BACK,
                .loadOp = LoadOp::LOAD_OP_DONT_CARE,
            },
            AttachmentDesc{
                .name   = "default_depth",
                .target = GL_DEPTH,
                .slot   = GL_DEPTH_ATTACHMENT,
                .loadOp = LoadOp::LOAD_OP_DONT_CARE,  // never clear depth for skybox pass
            },
        },
    };

    // 2. Initialize pipeline states for each renderpass, namely configure the fixed-function stages including rasterization/blend/depth/stencil
    m_states["deferred_geometry"] = PipelineState{
        .polygonMode      = GL_FILL,
        .cullEnable       = GL_FALSE,
        .cullMode         = GL_BACK,
        .frontFace        = GL_CCW,
        .blendEnable      = GL_FALSE,
        .depthTestEnable  = GL_TRUE,
        .depthWriteEnable = GL_TRUE,
        .depthFunc        = GL_LESS,
    };
    m_states["deferred_shading"] = PipelineState{
        .polygonMode      = GL_FILL,
        .cullEnable       = GL_FALSE,
        .cullMode         = GL_BACK,
        .frontFace        = GL_CCW,
        .blendEnable      = GL_FALSE,
        .depthTestEnable  = GL_FALSE,
        .depthWriteEnable = GL_FALSE,
        .depthFunc        = GL_LESS,
    };
    m_states["forward_opaque"] = PipelineState{
        .polygonMode      = GL_FILL,
        .cullEnable       = GL_FALSE,
        .cullMode         = GL_BACK,
        .frontFace        = GL_CCW,
        .blendEnable      = GL_FALSE,
        .depthTestEnable  = GL_TRUE,
        .depthWriteEnable = GL_TRUE,
        .depthFunc        = GL_LESS,
    };
    m_states["skybox"] = PipelineState{
        .polygonMode      = GL_FILL,
        .cullEnable       = GL_FALSE,
        .cullMode         = GL_BACK,
        .frontFace        = GL_CCW,
        .blendEnable      = GL_FALSE,
        .depthTestEnable  = GL_TRUE,
        .depthWriteEnable = GL_FALSE,
        .depthFunc        = GL_LEQUAL,
    };

    // 3. Compile and link shaders
    m_shaders["deferred_geometry"] = std::make_shared<Shader>("../asset/shader/deferred_geometry.vert", "../asset/shader/deferred_geometry.frag");
    m_shaders["deferred_shading"]  = std::make_shared<Shader>("../asset/shader/deferred_shading.vert", "../asset/shader/deferred_shading.frag");
    m_shaders["forward_opaque"]    = std::make_shared<Shader>("../asset/shader/forward_opaque.vert", "../asset/shader/forward_opaque.frag");
    // m_shaders["forward_transparent"] = std::make_shared<Shader>("../asset/shader/forward_transparent.vert", "../asset/shader/forward_transparent.frag");
    m_shaders["skybox"] = std::make_shared<Shader>("../asset/shader/skybox.vert", "../asset/shader/skybox.frag");

    // 4. Create framebuffers
    m_frames["screen"]  = std::make_shared<FrameBuffer>(true, m_width, m_height);
    m_frames["gbuffer"] = std::make_shared<FrameBuffer>(false, m_width, m_height);

    // 5. Create textures and activate them as drawable attachments for framebuffers
    for (auto attachment : m_passes["deferred_geometry"].attachments) {
        if (attachment.type == GL_TEXTURE_2D || attachment.type == GL_TEXTURE_CUBE_MAP) {
            m_textures["gbuffer." + attachment.name] = std::make_shared<Texture>(m_width, m_height, attachment.type, attachment.format);
            m_frames["gbuffer"]->attach(attachment.slot, m_textures["gbuffer." + attachment.name]);
        }
    }
    m_frames["gbuffer"]->finalize();
}

void Renderer::shutdown() {
    m_shaders.clear();
    m_frames.clear();
    m_uniforms.clear();
    m_textures.clear();
}

void Renderer::prepare(const Scene& scene) {
    // 1. Update uniform buffers with scene data, and bind them to shader binding points
    // 1.1 Light block
    const auto& lights = scene.getLights();
    if (m_uniforms["light"] == nullptr) {
        // TODO: add multiple light sources support
        m_uniforms["light"] = std::make_shared<UniformBuffer>(sizeof(LightBlock));
    }
    m_uniforms["light"]->upload(0, sizeof(LightBlock), &lights[0]->getLightBlock());
    m_uniforms["light"]->bind(0);

    // 1.2 Camera block
    const auto& camera = scene.getCamera();
    if (m_uniforms["camera"] == nullptr) {
        m_uniforms["camera"] = std::make_shared<UniformBuffer>(sizeof(CameraBlock));
    }
    m_uniforms["camera"]->upload(0, sizeof(CameraBlock), &camera->getCameraBlock());
    m_uniforms["camera"]->bind(1);

    // 1.3 Model block
    const auto& blocks = scene.getModelBlocks();
    if (m_uniforms["model"] == nullptr) {
        m_uniforms["model"] = std::make_shared<UniformBuffer>(sizeof(ModelBlock) * blocks.size());
    }
    m_uniforms["model"]->upload(0, sizeof(ModelBlock) * blocks.size(), blocks.data());
    m_uniforms["model"]->bind(2);

    // 2. Precalculate environment map
}

void Renderer::render(const Scene& scene) {
    StaticResource& instance = StaticResource::getInstance();
    std::vector<RenderItem> opaqueQueue, transparentQueue;
    scene.getRenderQueue(opaqueQueue, true);
    scene.getRenderQueue(transparentQueue, false);
    // std::cout << "OpaqueQueue size: " << opaqueQueue.size() << std::endl;
    // std::cout << "TransparentQueue size: " << transparentQueue.size() << std::endl;

    {
        m_states["deferred_geometry"].apply();
        m_shaders["deferred_geometry"]->use();
        m_passes["deferred_geometry"].begin(m_frames["gbuffer"]);
        for (const auto& item : opaqueQueue) {
            m_uniforms["model"]->bind(2, item.uoffset, sizeof(ModelBlock));
            draw(item);
        }
        m_passes["deferred_geometry"].end();
    }

    {
        //! WARNING: Copy depth buffer to screen framebuffer, otherwise depth test will fail for subsequent passes that bind screen framebuffer, since gbuffer's depth buffer is not shared with screen framebuffer. This is a workaround for the fact that OpenGL does not support framebuffer inheritance and subpasses like Vulkan, which allow multiple passes to share the same depth attachment without copying.
        m_frames["screen"]->copy(*m_frames["gbuffer"], GL_DEPTH_BUFFER_BIT);

        m_states["deferred_shading"].apply();
        m_shaders["deferred_shading"]->use();
        m_passes["deferred_shading"].begin(m_frames["screen"]);
        draw(
            instance.getLayout("quad"),
            instance.getCounts("quad"),
            {
                m_textures["gbuffer.albedo"],
                m_textures["gbuffer.normal"],
                m_textures["gbuffer.mrao"],
                m_textures["gbuffer.depth"],
            },
            4
        );
        m_passes["deferred_shading"].end();
    }

    std::vector<float> depth(m_width * m_height);
    m_frames["screen"]->read(GL_DEPTH, GL_DEPTH_ATTACHMENT, depth, GL_DEPTH_COMPONENT32);
    for (int i = 0; i < depth.size(); i++) {
        if (depth[i] < 1.0f && depth[i] > 0.0f) {
            std::cout << "Depth at pixel " << i << ": " << depth[i] << std::endl;
        }
    }

    // {
    //     m_states["forward_opaque"].apply();
    //     m_shaders["forward_opaque"]->use();
    //     m_passes["forward_opaque"].begin(m_frames["screen"]);
    //     for (const auto& item : opaqueQueue) {
    //         m_uniforms["model"]->bind(2, item.uoffset, sizeof(ModelBlock));
    //         draw(item);
    //     }
    //     m_passes["forward_opaque"].end();
    // }

    {
        m_states["skybox"].apply();
        m_shaders["skybox"]->use();
        m_passes["skybox"].begin(m_frames["screen"]);
        draw(
            instance.getLayout("skybox"),
            instance.getCounts("skybox"),
            {
                scene.getSkybox(),
            },
            8
        );
        m_passes["skybox"].end();
    }
}

void Renderer::draw(const RenderItem& item) {
    const std::shared_ptr<VertexLayout>& layout  = item.mesh->getVertexLayout();
    const std::unique_ptr<VertexBuffer>& bufferv = item.mesh->getVertexBuffer();
    const std::unique_ptr<IndexBuffer>& bufferi  = item.mesh->getIndexBuffer();

    item.material->getAlbedoMap()->bind(0);
    item.material->getNormalMap()->bind(1);
    item.material->getMRAOMap()->bind(2);

    layout->bind();
    if (!layout->attach(0, bufferv, 0, sizeof(Vertex))) {  //! WARNING: slot 0 and stride sizeof(Vertex) are hardcoded since all meshes share the same vertex format, but can be easily extended in the future if needed
        return;
    }
    if (layout->attach(bufferi)) {
        // std::cout << "Mesh IBO Draw: index count " << item.length << " offset " << item.ioffset << std::endl;
        // !WARNING: The last parameter is the index buffer offset in bytes.
        glDrawElements(GL_TRIANGLES, item.length, GL_UNSIGNED_INT, (void*)(sizeof(GLuint) * item.ioffset));
    } else {
        // std::cout << "Mesh VBO Draw: vertex count " << item.length << " offset " << item.ioffset << std::endl;
        // !WARNING: The second parameter is vertex offset in vertex count, not bytes.
        glDrawArrays(GL_TRIANGLES, item.ioffset, item.length);
    }
}

void Renderer::draw(const std::shared_ptr<VertexLayout>& layout, uint32_t vertexCount, const std::vector<std::shared_ptr<Texture>>& textures, uint32_t startSlot) {
    layout->bind();
    for (int i = 0; i < textures.size(); i++) {
        if (textures[i] == nullptr) {
            return;
        }
        textures[i]->bind(i + startSlot);
    }
    // std::cout << "Quad/Skybox VBO Draw: vertex count " << vertexCount << std::endl;
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    return;
}

}  // namespace tinyrenderer

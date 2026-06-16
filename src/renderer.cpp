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
#include "shaderstoragebuffer.hpp"
#include "staticresource.hpp"
#include "uniformbuffer.hpp"
#include "utils.hpp"

namespace tinyrenderer {

void Renderer::setup() {
    // 1. Initialize renderpasses, namely define the input and output attachments of each pipeline
    m_passes["shadow_mapping"] = RenderPass{
        .attachments = {
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
    m_states["shadow_mapping"] = PipelineState{
        .viewPortDynamic  = GL_TRUE,
        .depthTestEnable  = GL_TRUE,
        .depthWriteEnable = GL_TRUE,
        .depthFunc        = GL_LESS,
    };
    m_states["deferred_geometry"] = PipelineState{
        .viewX            = 0,
        .viewY            = 0,
        .viewW            = (GLsizei)m_width,
        .viewH            = (GLsizei)m_height,
        .depthTestEnable  = GL_TRUE,
        .depthWriteEnable = GL_TRUE,
        .depthFunc        = GL_LESS,
    };
    m_states["deferred_shading"] = PipelineState{
        .viewX            = 0,
        .viewY            = 0,
        .viewW            = (GLsizei)m_width,
        .viewH            = (GLsizei)m_height,
        .depthTestEnable  = GL_FALSE,
        .depthWriteEnable = GL_FALSE,
        .depthFunc        = GL_LESS,
    };
    m_states["forward_opaque"] = PipelineState{
        .viewX            = 0,
        .viewY            = 0,
        .viewW            = (GLsizei)m_width,
        .viewH            = (GLsizei)m_height,
        .depthTestEnable  = GL_TRUE,
        .depthWriteEnable = GL_TRUE,
        .depthFunc        = GL_LESS,
    };
    m_states["skybox"] = PipelineState{
        .viewX            = 0,
        .viewY            = 0,
        .viewW            = (GLsizei)m_width,
        .viewH            = (GLsizei)m_height,
        .depthTestEnable  = GL_TRUE,
        .depthWriteEnable = GL_FALSE,
        .depthFunc        = GL_LEQUAL,
    };

    // 3. Compile and link shaders
    m_shaders["shadow_mapping"]      = std::make_shared<Shader>("../asset/shader/shadow_mapping.vert", "../asset/shader/shadow_mapping.frag");
    m_shaders["deferred_geometry"]   = std::make_shared<Shader>("../asset/shader/deferred_geometry.vert", "../asset/shader/deferred_geometry.frag");
    m_shaders["deferred_shading"]    = std::make_shared<Shader>("../asset/shader/deferred_shading.vert", "../asset/shader/deferred_shading.frag");
    m_shaders["forward_opaque"]      = std::make_shared<Shader>("../asset/shader/forward_opaque.vert", "../asset/shader/forward_opaque.frag");
    m_shaders["forward_transparent"] = std::make_shared<Shader>("../asset/shader/forward_transparent.vert", "../asset/shader/forward_transparent.frag");
    m_shaders["skybox"]              = std::make_shared<Shader>("../asset/shader/skybox.vert", "../asset/shader/skybox.frag");

    // 4. Create framebuffers
    m_frames["screen"]  = std::make_shared<FrameBuffer>(true, m_width, m_height);
    m_frames["shadow"]  = std::make_shared<FrameBuffer>(false, m_shadowMapWidth, m_shadowMapHeight);
    m_frames["gbuffer"] = std::make_shared<FrameBuffer>(false, m_width, m_height);

    // 5. Create textures and activate them as drawable attachments for framebuffers
    std::unordered_map<std::string, std::string> pass_2_frame = {
        {"shadow_mapping", "shadow"},
        {"deferred_geometry", "gbuffer"},
    };

    for (const auto& [passName, pass] : m_passes) {
        if (pass_2_frame.count(passName) == 0) {
            continue;
        }
        const auto& frameName = pass_2_frame[passName];
        std::cout << "Creating attachments [";
        for (auto attachment : pass.attachments) {
            auto attchName = frameName + "." + attachment.name;

            std::cout << attchName << ", ";
            m_textures[attchName] = std::make_shared<Texture>(m_frames[frameName]->getWidth(), m_frames[frameName]->getHeight(), attachment.type, attachment.format);
            m_frames[frameName]->attach(attachment.slot, m_textures[attchName]);
        }
        std::cout << "]\n";
        m_frames[frameName]->finalize();
        m_frames[frameName]->validate();
    }

    // 6. Create samplers for corresponding textures
    // | Slot | Macro        | Texture Name    |
    // |------|--------------|-----------------|
    // | 0    | GL_TEXTURE0  | material.albedo |
    // | 1    | GL_TEXTURE1  | material.normal |
    // | 2    | GL_TEXTURE2  | material.maro   |
    // | 3    | GL_TEXTURE3  | gbuffer.albedo  |
    // | 4    | GL_TEXTURE4  | gbuffer.normal  |
    // | 5    | GL_TEXTURE5  | gbuffer.maro    |
    // | 6    | GL_TEXTURE6  | gbuffer.depth   |
    // | 7    | GL_TEXTURE7  | shadow.depth    |
    // | 8    | GL_TEXTURE8  | scene.skybox    |
    m_samplers[0x0007] = std::make_shared<Sampler>(SamplerDesc{
        // slot 0, 1, 2
        .minFilter = GL_LINEAR_MIPMAP_LINEAR,
        .magFilter = GL_LINEAR,
        .wrapS     = GL_REPEAT,
        .wrapT     = GL_REPEAT,
    });
    m_samplers[0x0078] = std::make_shared<Sampler>(SamplerDesc{
        // slot 3, 4, 5, 6
        .minFilter = GL_NEAREST,
        .magFilter = GL_NEAREST,
        .wrapS     = GL_CLAMP_TO_EDGE,
        .wrapT     = GL_CLAMP_TO_EDGE,
    });
    m_samplers[0x0080] = std::make_shared<Sampler>(SamplerDesc{
        // slot 7
        .minFilter   = GL_NEAREST,
        .magFilter   = GL_NEAREST,
        .wrapS       = GL_CLAMP_TO_BORDER,
        .wrapT       = GL_CLAMP_TO_BORDER,
        .borderColor = {1.0f, 1.0f, 1.0f, 1.0f},
    });
    m_samplers[0x0100] = std::make_shared<Sampler>(SamplerDesc{
        // slot 8
        .minFilter = GL_LINEAR,
        .magFilter = GL_LINEAR,
        .wrapS     = GL_CLAMP_TO_EDGE,
        .wrapT     = GL_CLAMP_TO_EDGE,
        .wrapR     = GL_CLAMP_TO_EDGE,
    });
    for (auto& [slotMask, sampler] : m_samplers) {
        for (uint32_t slot = 0; slot < 9; slot++) {
            if (slotMask & (1 << slot)) {
                sampler->bind(slot);
            }
        }
    }
}

void Renderer::shutdown() {
    m_shaders.clear();
    m_frames.clear();
    m_buffers.clear();
    m_samplers.clear();
    m_textures.clear();
}

void Renderer::prepare(const Scene& scene) {
    // 1. Update uniform/shaderstorage buffers with scene data, and bind them to shader binding points
    // 1.1 Camera uniform block
    const auto& camera = scene.getCamera();
    if (m_buffers["camera"] == nullptr) {
        m_buffers["camera"] = std::make_shared<UniformBuffer>(sizeof(CameraBlock));
    }
    m_buffers["camera"]->upload(0, sizeof(CameraBlock), &camera->getCameraBlock());
    m_buffers["camera"]->bind(0);

    // 1.2 Model uniform block
    std::vector<ModelBlock> modelBlocks;
    scene.getModelBlocks(modelBlocks);
    if (m_buffers["model"] == nullptr) {
        m_buffers["model"] = std::make_shared<UniformBuffer>(sizeof(ModelBlock) * modelBlocks.size());
    }
    m_buffers["model"]->upload(0, sizeof(ModelBlock) * modelBlocks.size(), modelBlocks.data());
    m_buffers["model"]->bind(1);

    // 1.3 Light shader storage array
    std::vector<LightBlock> lightBlocks;
    scene.getLightBlocks(lightBlocks);
    if (m_buffers["light"] == nullptr) {
        m_buffers["light"] = std::make_shared<ShaderStorageBuffer>(sizeof(LightBlock) * lightBlocks.size());
    }
    m_buffers["light"]->upload(0, sizeof(LightBlock) * lightBlocks.size(), lightBlocks.data());
    m_buffers["light"]->bind(0);

    // 2. Generate render item queue(draw command)
    std::vector<RenderItem> opaqueQueue, transparentQueue;
    scene.getRenderQueue(opaqueQueue, true);
    scene.getRenderQueue(transparentQueue, false);

    // 3. Precalculate environment map
    // TODO: add imaged based light support
    if (m_enableIBL) {
    }

    // 4. Render shadow map and update the light shader storage buffer if needed
    if (m_enableShadow) {
        std::vector<std::shared_ptr<Light>> lights = scene.getLights();
        std::vector<int> rects;
        std::vector<float> remaps;

        m_frames["shadow"]->divide(rects, remaps, lights.size());
        m_states["shadow_mapping"].apply();
        m_passes["shadow_mapping"].begin(m_frames["shadow"]);
        m_shaders["shadow_mapping"]->use();
        for (int i = 0; i < lights.size(); i++) {
            m_states["shadow_mapping"].view(rects[i * 4], rects[i * 4 + 1], rects[i * 4 + 2], rects[i * 4 + 3]);
            m_shaders["shadow_mapping"]->setUniformValue("uLightViewProjMatrix", lights[i]->getViewProjMatrix());
            for (const auto& item : opaqueQueue) {
                m_buffers["model"]->bind(1, item.uoffset, sizeof(ModelBlock));
                draw(item);
            }
            lights[i]->setUVOffsetScale({remaps[i * 4], remaps[i * 4 + 1]}, {remaps[i * 4 + 2], remaps[i * 4 + 3]});
        }
        m_passes["shadow_mapping"].end();

        scene.getLightBlocks(lightBlocks);
        m_buffers["light"]->upload(0, sizeof(LightBlock) * lightBlocks.size(), lightBlocks.data());
    } else {
        float depth = 1.0f;
        m_textures["shadow.depth"]->clear(&depth, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    }
}

void Renderer::render(const Scene& scene) {
    StaticResource& instance = StaticResource::getInstance();
    std::vector<RenderItem> opaqueQueue, transparentQueue;
    scene.getRenderQueue(opaqueQueue, true);
    scene.getRenderQueue(transparentQueue, false);
    // std::cout << "OpaqueQueue size: " << opaqueQueue.size() << std::endl;
    // std::cout << "TransparentQueue size: " << transparentQueue.size() << std::endl;

    // {
    //     m_states["deferred_geometry"].apply();
    //     m_shaders["deferred_geometry"]->use();
    //     m_passes["deferred_geometry"].begin(m_frames["gbuffer"]);
    //     for (const auto& item : opaqueQueue) {
    //         m_buffers["model"]->bind(1, item.uoffset, sizeof(ModelBlock));
    //         draw(item);
    //     }
    //     m_passes["deferred_geometry"].end();
    // }

    // {
    //     //! WARNING: Copy depth buffer to screen framebuffer, otherwise depth test will fail for subsequent passes that bind screen framebuffer, since gbuffer's depth buffer is not shared with screen framebuffer.(e.g., skybox will fail if copy is commented) This is a workaround for the fact that OpenGL does not support framebuffer inheritance and subpasses like Vulkan, which allow multiple passes to share the same depth attachment without copying.
    //     m_frames["screen"]->copy(*m_frames["gbuffer"], GL_DEPTH_BUFFER_BIT);

    //     m_states["deferred_shading"].apply();
    //     m_passes["deferred_shading"].begin(m_frames["screen"]);
    //     m_shaders["deferred_shading"]->use();
    //     m_shaders["deferred_shading"]->setUniformValue("uLightCount", (int)scene.getLights().size());
    //     draw(
    //         instance.getLayout("quad"),
    //         instance.getCounts("quad"),
    //         {
    //             m_textures["gbuffer.albedo"],
    //             m_textures["gbuffer.normal"],
    //             m_textures["gbuffer.mrao"],
    //             m_textures["gbuffer.depth"],
    //             m_textures["shadow.depth"],
    //         },
    //         3
    //     );
    //     m_passes["deferred_shading"].end();
    // }

    {
        m_states["forward_opaque"].apply();
        m_passes["forward_opaque"].begin(m_frames["screen"]);
        m_shaders["forward_opaque"]->use();
        m_shaders["forward_opaque"]->setUniformValue("uLightCount", (int)scene.getLights().size());
        for (const auto& item : opaqueQueue) {
            m_buffers["model"]->bind(1, item.uoffset, sizeof(ModelBlock));
            m_textures["shadow.depth"]->bind(7);
            draw(item);
        }
        m_passes["forward_opaque"].end();
    }

    if (scene.getSkybox()) {
        m_states["skybox"].apply();
        m_passes["skybox"].begin(m_frames["screen"]);
        m_shaders["skybox"]->use();
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

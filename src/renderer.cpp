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
    // 0. Define name mappings
    m_pass2FrameNames = {
        {"equirect_to_cubemap", "skybox"},
        {"shadow_mapping", "shadow"},
        {"deferred_geometry", "gbuffer"},
        {"deferred_shading", "screen"},
        {"forward_opaque", "screen"},
        {"skybox", "screen"},
    };
    m_texture2SlotIndexs = {
        // 0~8: material textures
        {"albedo", 0},
        {"normal", 1},
        {"mrao", 2},

        // 9~15: gbuffer textures
        {"gbuffer.albedo", 9},
        {"gbuffer.normal", 10},
        {"gbuffer.mrao", 11},
        {"gbuffer.depth", 12},

        // 16~19: shadow textures
        {"shadow.basic", 16},
        {"shadow.cascade", 17},

        // 20~23: environment/ibl textures
        {"skybox.cubemap", 20},
        {"skybox.equirect", 21},
        {"ibl.diffuse", 22},
        {"ibl.specular", 23},
    };

    // 1. Initialize renderpasses, namely define the input and output attachments of each pipeline
    m_passes["equirect_to_cubemap"] = RenderPass{
        .attachments = {
            AttachmentDesc{
                .name   = "cubemap",
                .target = GL_COLOR,
                .type   = GL_TEXTURE_CUBE_MAP,
                .format = GL_RGB32F,
                .slot   = GL_COLOR_ATTACHMENT0,
                .loadOp = LoadOp::LOAD_OP_CLEAR,
                .value  = {.color = {0.0f, 0.0f, 0.0f, 1.0f}},
            },
        },
    };
    m_passes["shadow_mapping"] = RenderPass{
        .attachments = {
            AttachmentDesc{
                .name   = "basic",
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
    m_states["equirect_to_cubemap"] = PipelineState{
        .viewX            = 0,
        .viewY            = 0,
        .viewW            = (GLsizei)m_skyboxSize,
        .viewH            = (GLsizei)m_skyboxSize,
        .depthTestEnable  = GL_FALSE,
        .depthWriteEnable = GL_FALSE,
        .depthFunc        = GL_LEQUAL,
    };
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
    m_shaders["equirect_to_cubemap"] = std::make_shared<Shader>("../asset/shader/equirect_to_cubemap.vert", "../asset/shader/equirect_to_cubemap.frag");
    m_shaders["shadow_mapping"]      = std::make_shared<Shader>("../asset/shader/shadow_mapping.vert", "../asset/shader/shadow_mapping.frag");
    m_shaders["deferred_geometry"]   = std::make_shared<Shader>("../asset/shader/deferred_geometry.vert", "../asset/shader/deferred_geometry.frag");
    m_shaders["deferred_shading"]    = std::make_shared<Shader>("../asset/shader/deferred_shading.vert", "../asset/shader/deferred_shading.frag");
    m_shaders["forward_opaque"]      = std::make_shared<Shader>("../asset/shader/forward_opaque.vert", "../asset/shader/forward_opaque.frag");
    m_shaders["skybox"]              = std::make_shared<Shader>("../asset/shader/skybox.vert", "../asset/shader/skybox.frag");

    // 4. Create framebuffers
    m_frames["screen"]  = std::make_shared<FrameBuffer>(true, m_width, m_height);
    m_frames["shadow"]  = std::make_shared<FrameBuffer>(false, m_shadowMapWidth, m_shadowMapHeight);
    m_frames["skybox"]  = std::make_shared<FrameBuffer>(false, m_skyboxSize, m_skyboxSize);
    m_frames["gbuffer"] = std::make_shared<FrameBuffer>(false, m_width, m_height);

    // 5. Create textures and activate them as drawable attachments for framebuffers
    for (const auto& [passName, pass] : m_passes) {
        if (m_pass2FrameNames.count(passName) == 0) {
            continue;
        }
        const auto& frameName = m_pass2FrameNames[passName];
        if (frameName == "screen") {
            continue;
        }
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

    // 6. Create samplers for corresponding slots
    m_samplers[0x00FF]   = std::make_shared<Sampler>(SamplerDesc{
        // slot 0~7 -> GL_TEXTURE0~GL_TEXTURE7 = material textures
        .minFilter = GL_LINEAR_MIPMAP_LINEAR,
        .magFilter = GL_LINEAR,
        .wrapS     = GL_REPEAT,
        .wrapT     = GL_REPEAT,
    });
    m_samplers[0xFF00]   = std::make_shared<Sampler>(SamplerDesc{
        // slot 8~15 -> GL_TEXTURE8~GL_TEXTURE15 = gbuffer textures
        .minFilter = GL_NEAREST,
        .magFilter = GL_NEAREST,
        .wrapS     = GL_CLAMP_TO_EDGE,
        .wrapT     = GL_CLAMP_TO_EDGE,
    });
    m_samplers[0x0F0000] = std::make_shared<Sampler>(SamplerDesc{
        // slot 16~19 -> GL_TEXTURE16~GL_TEXTURE19 = shadow textures
        .minFilter   = GL_NEAREST,
        .magFilter   = GL_NEAREST,
        .wrapS       = GL_CLAMP_TO_BORDER,
        .wrapT       = GL_CLAMP_TO_BORDER,
        .borderColor = {1.0f, 1.0f, 1.0f, 1.0f},
    });
    m_samplers[0xF00000] = std::make_shared<Sampler>(SamplerDesc{
        // slot 20~23 -> GL_TEXTURE20~GL_TEXTURE23 = skybox textures
        .minFilter = GL_LINEAR,
        .magFilter = GL_LINEAR,
        .wrapS     = GL_CLAMP_TO_EDGE,
        .wrapT     = GL_CLAMP_TO_EDGE,
        .wrapR     = GL_CLAMP_TO_EDGE,
    });
    for (auto& [slotMask, sampler] : m_samplers) {
        for (uint32_t slot = 0; slot < 32; slot++) {
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
    {
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
    }

    // 2. Generate render item queue(draw command)
    StaticResource& instance = StaticResource::getInstance();
    std::vector<RenderItem> opaqueQueue, transparentQueue;
    scene.getRenderQueue(opaqueQueue, true);
    scene.getRenderQueue(transparentQueue, false);

    // 3. Render shadow map and update the light shader storage buffer if needed
    if (m_enableShadow) {
        std::vector<std::shared_ptr<Light>> lights = scene.getLights();
        std::vector<int> rects;
        std::vector<float> remaps;

        m_frames["shadow"]->divide(rects, remaps, lights.size());
        m_states["shadow_mapping"].apply();
        m_shaders["shadow_mapping"]->use();
        m_passes["shadow_mapping"].begin(m_frames["shadow"]);
        for (int i = 0; i < lights.size(); i++) {
            m_states["shadow_mapping"].view(rects[i * 4], rects[i * 4 + 1], rects[i * 4 + 2], rects[i * 4 + 3]);
            m_shaders["shadow_mapping"]->setUniformValue("uLightViewProjMatrix", lights[i]->getViewProjMatrix());
            for (const auto& item : opaqueQueue) {
                m_buffers["model"]->bind(1, item.uoffset, sizeof(ModelBlock));
                draw(item, {});
            }
            lights[i]->setUVOffsetScale({remaps[i * 4], remaps[i * 4 + 1]}, {remaps[i * 4 + 2], remaps[i * 4 + 3]});
        }
        m_passes["shadow_mapping"].end();

        std::vector<LightBlock> lightBlocks;
        scene.getLightBlocks(lightBlocks);
        m_buffers["light"]->upload(0, sizeof(LightBlock) * lightBlocks.size(), lightBlocks.data());
    } else {
        float depth = 1.0f;
        m_textures["shadow.basic"]->clear(&depth, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    }

    // 4. Convert equirect skybox into cube map skybox if needed
    const auto& cubemap           = scene.getSkyboxCubeMap();
    m_textures["skybox.equirect"] = scene.getSkyboxEquirect();
    if (cubemap == nullptr && m_textures["skybox.equirect"] != nullptr) {
        std::cout << "Converting equirect skybox into cube map skybox..." << std::endl;

        m_states["equirect_to_cubemap"].apply();
        m_shaders["equirect_to_cubemap"]->use();

        for (GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X; face <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z; face++) {
            GLint index   = face - GL_TEXTURE_CUBE_MAP_POSITIVE_X;
            GLsizei count = instance.getCounts("skybox");
            auto& layout  = instance.getLayout("skybox");

            m_frames["skybox"]->attach(GL_COLOR_ATTACHMENT0, m_textures["skybox.cubemap"], 0, index);
            m_shaders["equirect_to_cubemap"]->setUniformValue("uViewProjMatrix", instance.getCaptureMatrix(index));
            m_passes["equirect_to_cubemap"].begin(m_frames["skybox"]);
            draw(layout, {"skybox.equirect"}, count);
            m_passes["equirect_to_cubemap"].end();
        }
    } else {
        std::cout << "Using existing cube map skybox." << std::endl;
        std::cout << cubemap << ' ' << m_textures["skybox.equirect"] << std::endl;
        m_textures["skybox.cubemap"] = scene.getSkyboxCubeMap();
    }

    // 5. Precalculate environment map
    // TODO: add imaged based light support
    if (m_enableIBL) {
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
    //         draw(item, {"albedo", "normal", "mrao"});
    //     }
    //     m_passes["deferred_geometry"].end();
    // }

    //! WARNING: Copy depth buffer to screen framebuffer, otherwise depth test will fail for subsequent passes that bind screen framebuffer, since gbuffer's depth buffer is not shared with screen framebuffer.(e.g., skybox will fail if copy is commented) This is a workaround for the fact that OpenGL does not support framebuffer inheritance and subpasses like Vulkan, which allow multiple passes to share the same depth attachment without copying.
    // m_frames["screen"]->copy(*m_frames["gbuffer"], GL_DEPTH_BUFFER_BIT);

    // {
    //     GLsizei count = instance.getCounts("quad");
    //     auto& layout  = instance.getLayout("quad");

    //     m_states["deferred_shading"].apply();
    //     m_shaders["deferred_shading"]->use();
    //     m_passes["deferred_shading"].begin(m_frames["screen"]);
    //     m_shaders["deferred_shading"]->setUniformValue("uLightCount", (int)scene.getLights().size());
    //     draw(layout, {"gbuffer.albedo", "gbuffer.normal", "gbuffer.mrao", "shadow.basic"}, count);
    //     m_passes["deferred_shading"].end();
    // }

    {
        m_states["forward_opaque"].apply();
        m_shaders["forward_opaque"]->use();
        m_passes["forward_opaque"].begin(m_frames["screen"]);
        m_shaders["forward_opaque"]->setUniformValue("uLightCount", (int)scene.getLights().size());
        for (const auto& item : opaqueQueue) {
            m_buffers["model"]->bind(1, item.uoffset, sizeof(ModelBlock));
            draw(item, {"albedo", "normal", "mrao", "shadow.basic"});
        }
        m_passes["forward_opaque"].end();
    }

    if (m_textures["skybox.cubemap"] != nullptr) {
        GLsizei count = instance.getCounts("skybox");
        auto& layout  = instance.getLayout("skybox");

        m_states["skybox"].apply();
        m_shaders["skybox"]->use();
        m_passes["skybox"].begin(m_frames["screen"]);
        draw(layout, {"skybox.cubemap"}, count);
        m_passes["skybox"].end();
    }
}

void Renderer::draw(const RenderItem& item, const std::vector<std::string>& textures) {
    const std::shared_ptr<VertexLayout>& layout  = item.mesh->getVertexLayout();
    const std::unique_ptr<VertexBuffer>& bufferv = item.mesh->getVertexBuffer();
    const std::unique_ptr<IndexBuffer>& bufferi  = item.mesh->getIndexBuffer();

    for (auto name : textures) {
        if (m_texture2SlotIndexs.count(name) == 0) {
            throw std::runtime_error("Renderer::draw: Texture slot index not found: " + name);
        }
        if (item.material->getTexture(name) == nullptr && (m_textures.count(name) == 0 || m_textures[name] == nullptr)) {
            throw std::runtime_error("Renderer::draw: Texture not found: " + name);
        }
        std::shared_ptr<Texture> texture = item.material->getTexture(name) != nullptr ? item.material->getTexture(name) : m_textures[name];
        texture->bind(m_texture2SlotIndexs[name]);
    }

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

void Renderer::draw(const std::shared_ptr<VertexLayout>& layout, const std::vector<std::string>& textures, GLsizei count) {
    layout->bind();
    for (auto name : textures) {
        if (m_texture2SlotIndexs.count(name) == 0) {
            throw std::runtime_error("Renderer::draw: Texture slot index not found: " + name);
        }
        if (m_textures.count(name) == 0) {
            throw std::runtime_error("Renderer::draw: Texture not found: " + name);
        }
        m_textures[name]->bind(m_texture2SlotIndexs[name]);
    }
    // std::cout << "Quad/Skybox VBO Draw: vertex count " << count << std::endl;
    glDrawArrays(GL_TRIANGLES, 0, count);
    return;
}

}  // namespace tinyrenderer

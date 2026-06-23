#include "renderer.hpp"

#include <format>
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
    {
        m_pass2FrameNames = {
            {"equirect_to_cubemap", "skybox"},
            {"ibl_irradiance", "ibl_diffuse"},
            {"ibl_prefiltered", "ibl_specular"},
            {"ibl_brdf_lut", "ibl_brdf_lut"},
            {"shadow_mapping", "shadow"},
            {"deferred_geometry", "gbuffer"},
            {"deferred_shading", "hdr_screen"},
            {"forward_opaque", "hdr_screen"},
            {"skybox", "hdr_screen"},
            {"postprocess_highlight", "highlight"},
            {"postprocess_kawase_down", "blur_down"},
            {"postprocess_kawase_up", "blur_up"},
            {"postprocess_lensflare", "lensflare"},
            {"postprocess_gaussian_blur", "blur_x"},
            {"postprocess_gaussian_blur", "blur_y"},
            {"postprocess_final", "screen"},
        };
        m_texture2SlotIndexs = {
            // 0~7: material textures
            {"albedo", 0},
            {"normal", 1},
            {"mrao", 2},

            // 8~11: gbuffer textures
            {"gbuffer.albedo", 8},
            {"gbuffer.normal", 9},
            {"gbuffer.mrao", 10},
            {"gbuffer.depth", 11},

            // 12~15: shadow textures
            {"shadow", 12},

            // 16~23: environment/ibl textures
            {"skybox.cubemap", 16},
            {"skybox.equirect", 17},  // skybox.equirect is registered in m_textures by scene.m_skyboxEquirect when prepare() is called
            {"ibl_diffuse", 18},
            {"ibl_specular", 19},
            {"ibl_brdf_lut", 20},

            // 24~31: postprocess textures
            {"hdr_screen.color", 24},
            {"hdr_screen.depth", -1},  // only used as output, no need to bind
            {"highlight", 25},
            {"blur_down", 26},
            {"blur_up", 27},
            {"bloom", 27},  // bloom is the output of bloom blur pass
            {"lensflare", 28},
            {"blur_x", 29},
            {"blur_y", 30},  // even though ping-pong buffering technique is used in blur pass, blur_x and blur_y can not share the same slot(write/read access may cause conflict)
            {"dirtmask", 31},
        };
    }

    // 1. Initialize renderpasses, namely define the input and output attachments of each pipeline
    {
        m_passes["equirect_to_cubemap"] = RenderPass{
            .attachments = {
                AttachmentDesc{
                    .name   = "cubemap",
                    .target = GL_COLOR,
                    .type   = GL_TEXTURE_CUBE_MAP,
                    .format = GL_RGBA32F,
                    .slot   = GL_COLOR_ATTACHMENT0,
                    .loadOp = LoadOp::LOAD_OP_CLEAR,
                    .value  = {.color = {0.0f, 0.0f, 0.0f, 1.0f}},
                },
            },
        };
        m_passes["ibl_irradiance"] = RenderPass{
            .attachments = {
                AttachmentDesc{
                    .name   = "",  // use frame buffer name as ouput attachment name
                    .target = GL_COLOR,
                    .type   = GL_TEXTURE_CUBE_MAP,
                    .format = GL_RGBA32F,
                    .slot   = GL_COLOR_ATTACHMENT0,
                    .loadOp = LoadOp::LOAD_OP_CLEAR,
                    .value  = {.color = {0.0f, 0.0f, 0.0f, 1.0f}},
                },
            },
        };
        m_passes["ibl_prefiltered"] = RenderPass{
            .attachments = {
                AttachmentDesc{
                    .name      = "",  // use frame buffer name as ouput attachment name
                    .target    = GL_COLOR,
                    .type      = GL_TEXTURE_CUBE_MAP,
                    .format    = GL_RGBA32F,
                    .slot      = GL_COLOR_ATTACHMENT0,
                    .mipLevels = 7,
                    .loadOp    = LoadOp::LOAD_OP_CLEAR,
                    .value     = {.color = {0.0f, 0.0f, 0.0f, 1.0f}},
                },
            },
        };
        m_passes["ibl_brdf_lut"] = RenderPass{
            .attachments = {
                AttachmentDesc{
                    .name   = "",  // use frame buffer name as ouput attachment name
                    .target = GL_COLOR,
                    .type   = GL_TEXTURE_2D,
                    .format = GL_RGBA32F,
                    .slot   = GL_COLOR_ATTACHMENT0,
                    .loadOp = LoadOp::LOAD_OP_CLEAR,
                    .value  = {.color = {0.0f, 0.0f, 0.0f, 1.0f}},
                },
            },
        };
        m_passes["shadow_mapping"] = RenderPass{
            .attachments = {
                AttachmentDesc{
                    .name   = "",  // use frame buffer name as ouput attachment name
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
                    .name   = "color",
                    .target = GL_COLOR,
                    .type   = GL_TEXTURE_2D,
                    .format = GL_RGBA32F,
                    .slot   = GL_COLOR_ATTACHMENT0,
                    .loadOp = LoadOp::LOAD_OP_CLEAR,
                    .value  = {.color = {0.0f, 0.0f, 0.0f, 1.0f}},
                },
                AttachmentDesc{
                    .name   = "depth",
                    .target = GL_DEPTH,
                    .type   = GL_TEXTURE_2D,
                    .format = GL_DEPTH_COMPONENT24,
                    .slot   = GL_DEPTH_ATTACHMENT,
                    .loadOp = LoadOp::LOAD_OP_DONT_CARE  // manually copy depth from deferred_geometry pass
                },
            },
        };
        m_passes["forward_opaque"] = RenderPass{
            .attachments = {
                AttachmentDesc{
                    .name   = "color",
                    .target = GL_COLOR,
                    .type   = GL_TEXTURE_2D,
                    .format = GL_RGBA32F,
                    .slot   = GL_COLOR_ATTACHMENT0,
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
        m_passes["skybox"] = RenderPass{
            .attachments = {
                AttachmentDesc{
                    .name   = "color",
                    .target = GL_COLOR,
                    .type   = GL_TEXTURE_2D,
                    .format = GL_RGBA32F,
                    .slot   = GL_COLOR_ATTACHMENT0,
                    .loadOp = LoadOp::LOAD_OP_DONT_CARE,
                },
                AttachmentDesc{
                    .name   = "depth",
                    .target = GL_DEPTH,
                    .type   = GL_TEXTURE_2D,
                    .format = GL_DEPTH_COMPONENT24,
                    .slot   = GL_DEPTH_ATTACHMENT,
                    .loadOp = LoadOp::LOAD_OP_DONT_CARE,  // never clear depth for skybox pass
                },
            },
        };
        m_passes["postprocess_highlight"] = RenderPass{
            .attachments = {
                AttachmentDesc{
                    .name   = "",  // use frame buffer name as ouput attachment name
                    .target = GL_COLOR,
                    .type   = GL_TEXTURE_2D,
                    .format = GL_RGBA32F,
                    .slot   = GL_COLOR_ATTACHMENT0,
                    .loadOp = LoadOp::LOAD_OP_CLEAR,
                    .value  = {.color = {0.0f, 0.0f, 0.0f, 1.0f}},
                },
            },
        };
        m_passes["postprocess_kawase_down"] = RenderPass{
            .attachments = {
                AttachmentDesc{
                    .name      = "",  // use frame buffer name as ouput attachment name
                    .target    = GL_COLOR,
                    .type      = GL_TEXTURE_2D,
                    .format    = GL_RGBA32F,
                    .slot      = GL_COLOR_ATTACHMENT0,
                    .mipLevels = (GLsizei)m_bloomMipLevels,
                    .loadOp    = LoadOp::LOAD_OP_DONT_CARE,  // never clear color for kawase_down pass, otherwise deadlock may occur in mipmap pyramid downsampling
                },
            },
        };
        m_passes["postprocess_kawase_up"] = RenderPass{
            .attachments = {
                AttachmentDesc{
                    .name      = "",  // use frame buffer name as ouput attachment name
                    .target    = GL_COLOR,
                    .type      = GL_TEXTURE_2D,
                    .format    = GL_RGBA32F,
                    .slot      = GL_COLOR_ATTACHMENT0,
                    .mipLevels = (GLsizei)m_bloomMipLevels,
                    .loadOp    = LoadOp::LOAD_OP_DONT_CARE,  // never clear color for kawase_up pass, otherwise deadlock may occur in mipmap pyramid upsampling
                },
            },
        };
        m_passes["postprocess_lensflare"] = RenderPass{
            .attachments = {
                AttachmentDesc{
                    .name   = "",  // use frame buffer name as ouput attachment name
                    .target = GL_COLOR,
                    .type   = GL_TEXTURE_2D,
                    .format = GL_RGBA32F,
                    .slot   = GL_COLOR_ATTACHMENT0,
                    .loadOp = LoadOp::LOAD_OP_CLEAR,
                    .value  = {.color = {0.0f, 0.0f, 0.0f, 1.0f}},
                },
            },
        };
        m_passes["postprocess_gaussian_blur"] = RenderPass{
            .attachments = {
                AttachmentDesc{
                    .name   = "",  // use frame buffer name as ouput attachment name
                    .target = GL_COLOR,
                    .type   = GL_TEXTURE_2D,
                    .format = GL_RGBA32F,
                    .slot   = GL_COLOR_ATTACHMENT0,
                    .loadOp = LoadOp::LOAD_OP_DONT_CARE,
                },
            },
        };
        m_passes["postprocess_final"] = RenderPass{
            .attachments = {
                AttachmentDesc{
                    .name   = "default_color",  // screen color
                    .target = GL_COLOR,
                    .slot   = GL_BACK,
                    .loadOp = LoadOp::LOAD_OP_CLEAR,
                    .value  = {.color = {0.0f, 0.0f, 0.0f, 1.0f}},
                },
                AttachmentDesc{
                    .name   = "default_depth",  // screen depth
                    .target = GL_DEPTH,
                    .slot   = GL_DEPTH_ATTACHMENT,
                    .loadOp = LoadOp::LOAD_OP_DONT_CARE,
                },
            },
        };
    }

    // 2. Initialize pipeline states for each renderpass, namely configure the fixed-function stages including rasterization/blend/depth/stencil
    {
        m_states["equirect_to_cubemap"] = PipelineState{
            .viewX            = 0,
            .viewY            = 0,
            .viewW            = (GLsizei)m_skyboxSize,
            .viewH            = (GLsizei)m_skyboxSize,
            .depthTestEnable  = GL_FALSE,
            .depthWriteEnable = GL_FALSE,
        };
        m_states["ibl_irradiance"] = PipelineState{
            .viewX            = 0,
            .viewY            = 0,
            .viewW            = (GLsizei)m_skyboxSize,
            .viewH            = (GLsizei)m_skyboxSize,
            .depthTestEnable  = GL_FALSE,
            .depthWriteEnable = GL_FALSE,
        };
        m_states["ibl_prefiltered"] = PipelineState{
            .viewX            = 0,
            .viewY            = 0,
            .viewW            = (GLsizei)m_skyboxSize,
            .viewH            = (GLsizei)m_skyboxSize,
            .depthTestEnable  = GL_FALSE,
            .depthWriteEnable = GL_FALSE,
        };
        m_states["ibl_brdf_lut"] = PipelineState{
            .viewX            = 0,
            .viewY            = 0,
            .viewW            = (GLsizei)m_brdfLUTSize,
            .viewH            = (GLsizei)m_brdfLUTSize,
            .depthTestEnable  = GL_FALSE,
            .depthWriteEnable = GL_FALSE,
        };
        m_states["shadow_mapping"] = PipelineState{
            .viewportDynamic  = GL_TRUE,
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
        m_states["postprocess_highlight"] = PipelineState{
            .viewX            = 0,
            .viewY            = 0,
            .viewW            = (GLsizei)m_highlightMapSize,
            .viewH            = (GLsizei)m_highlightMapSize,
            .depthTestEnable  = GL_FALSE,
            .depthWriteEnable = GL_FALSE,
        };
        m_states["postprocess_kawase_down"] = PipelineState{
            .viewportDynamic  = GL_TRUE,
            .depthTestEnable  = GL_FALSE,
            .depthWriteEnable = GL_FALSE,
        };
        m_states["postprocess_kawase_up"] = PipelineState{
            .viewportDynamic  = GL_TRUE,
            .depthTestEnable  = GL_FALSE,
            .depthWriteEnable = GL_FALSE,
        };
        m_states["postprocess_lensflare"] = PipelineState{
            .viewX            = 0,
            .viewY            = 0,
            .viewW            = (GLsizei)m_lensflareMapSize,
            .viewH            = (GLsizei)m_lensflareMapSize,
            .depthTestEnable  = GL_FALSE,
            .depthWriteEnable = GL_FALSE,
        };
        m_states["postprocess_gaussian_blur"] = PipelineState{
            .viewX            = 0,
            .viewY            = 0,
            .viewW            = (GLsizei)m_lensflareMapSize,
            .viewH            = (GLsizei)m_lensflareMapSize,
            .depthTestEnable  = GL_FALSE,
            .depthWriteEnable = GL_FALSE,
        };
        m_states["postprocess_final"] = PipelineState{
            .viewportDynamic  = GL_TRUE,
            .depthTestEnable  = GL_FALSE,
            .depthWriteEnable = GL_FALSE,
        };
    }

    // 3. Compile and link shaders
    {
        m_shaders["equirect_to_cubemap"]       = std::make_shared<Shader>("../asset/shader/equirect_to_cubemap.vert", "../asset/shader/equirect_to_cubemap.frag");
        m_shaders["ibl_irradiance"]            = std::make_shared<Shader>("../asset/shader/ibl_irradiance.vert", "../asset/shader/ibl_irradiance.frag");
        m_shaders["ibl_prefiltered"]           = std::make_shared<Shader>("../asset/shader/ibl_prefiltered.vert", "../asset/shader/ibl_prefiltered.frag");
        m_shaders["ibl_brdf_lut"]              = std::make_shared<Shader>("../asset/shader/ibl_brdf_lut.vert", "../asset/shader/ibl_brdf_lut.frag");
        m_shaders["shadow_mapping"]            = std::make_shared<Shader>("../asset/shader/shadow_mapping.vert", "../asset/shader/shadow_mapping.frag");
        m_shaders["deferred_geometry"]         = std::make_shared<Shader>("../asset/shader/deferred_geometry.vert", "../asset/shader/deferred_geometry.frag");
        m_shaders["deferred_shading"]          = std::make_shared<Shader>("../asset/shader/deferred_shading.vert", "../asset/shader/deferred_shading.frag");
        m_shaders["forward_opaque"]            = std::make_shared<Shader>("../asset/shader/forward_opaque.vert", "../asset/shader/forward_opaque.frag");
        m_shaders["skybox"]                    = std::make_shared<Shader>("../asset/shader/skybox.vert", "../asset/shader/skybox.frag");
        m_shaders["postprocess_highlight"]     = std::make_shared<Shader>("../asset/shader/postprocess_highlight.vert", "../asset/shader/postprocess_highlight.frag");
        m_shaders["postprocess_kawase_down"]   = std::make_shared<Shader>("../asset/shader/postprocess_kawase_down.vert", "../asset/shader/postprocess_kawase_down.frag");
        m_shaders["postprocess_kawase_up"]     = std::make_shared<Shader>("../asset/shader/postprocess_kawase_up.vert", "../asset/shader/postprocess_kawase_up.frag");
        m_shaders["postprocess_lensflare"]     = std::make_shared<Shader>("../asset/shader/postprocess_lensflare.vert", "../asset/shader/postprocess_lensflare.frag");
        m_shaders["postprocess_gaussian_blur"] = std::make_shared<Shader>("../asset/shader/postprocess_gaussian_blur.vert", "../asset/shader/postprocess_gaussian_blur.frag");
        m_shaders["postprocess_final"]         = std::make_shared<Shader>("../asset/shader/postprocess.vert", "../asset/shader/postprocess.frag");
    }

    // 4. Create framebuffers
    {
        m_frames["ibl_diffuse"]  = std::make_shared<FrameBuffer>(false, m_skyboxSize, m_skyboxSize);
        m_frames["ibl_specular"] = std::make_shared<FrameBuffer>(false, m_skyboxSize, m_skyboxSize);
        m_frames["ibl_brdf_lut"] = std::make_shared<FrameBuffer>(false, m_brdfLUTSize, m_brdfLUTSize);
        m_frames["shadow"]       = std::make_shared<FrameBuffer>(false, m_shadowMapSize, m_shadowMapSize);
        m_frames["gbuffer"]      = std::make_shared<FrameBuffer>(false, m_width, m_height);
        m_frames["skybox"]       = std::make_shared<FrameBuffer>(false, m_skyboxSize, m_skyboxSize);
        m_frames["hdr_screen"]   = std::make_shared<FrameBuffer>(false, m_width, m_height);  // hdr_screen is the temporary frame buffer for shading pass, so that later can use it for postprocess(convert hdr into sdr/ldr)
        m_frames["highlight"]    = std::make_shared<FrameBuffer>(false, m_highlightMapSize, m_highlightMapSize);
        m_frames["blur_down"]    = std::make_shared<FrameBuffer>(false, m_bloomMapSize, m_bloomMapSize);
        m_frames["blur_up"]      = std::make_shared<FrameBuffer>(false, m_bloomMapSize, m_bloomMapSize);
        m_frames["lensflare"]    = std::make_shared<FrameBuffer>(false, m_lensflareMapSize, m_lensflareMapSize);
        m_frames["blur_x"]       = std::make_shared<FrameBuffer>(false, m_lensflareMapSize, m_lensflareMapSize);  // blur_x is the temporary frame buffer for horizontal gaussian blur
        m_frames["blur_y"]       = std::make_shared<FrameBuffer>(false, m_lensflareMapSize, m_lensflareMapSize);  // blur_y is the temporary frame buffer for vertical gaussian blur
        m_frames["screen"]       = std::make_shared<FrameBuffer>(true, m_width, m_height);
    }

    // 5. Create textures and activate them as drawable attachments for framebuffers
    for (const auto& [passName, pass] : m_passes) {
        if (m_pass2FrameNames.count(passName) == 0) {
            continue;
        }
        auto range = m_pass2FrameNames.equal_range(passName);
        for (auto itr = range.first; itr != range.second; ++itr) {
            auto frameName = itr->second;
            if (frameName == "screen") {
                continue;
            }
            std::cout << "Creating attachments [";
            for (auto attachment : pass.attachments) {
                auto attachmentName = attachment.name.empty() ? frameName : frameName + "." + attachment.name;
                std::cout << attachmentName << ", ";
                if (m_texture2SlotIndexs.count(attachmentName) == 0) {
                    throw std::runtime_error(format("Renderer::setup(): Attachment {} of pass {} not found in frame {}.", attachmentName, passName, frameName));
                }
                if (m_textures.count(attachmentName) == 0) {
                    m_textures[attachmentName] = std::make_shared<Texture>(m_frames[frameName]->getWidth(), m_frames[frameName]->getHeight(), attachment.type, attachment.format, attachment.mipLevels);
                }
                m_frames[frameName]->attach(attachment.slot, m_textures[attachmentName]);  // attach texture level 0 to frame buffer
            }
            std::cout << "]\n";
            m_frames[frameName]->finalize();
            m_frames[frameName]->validate();
        }
    }

    {
        std::cout << "Creating textures [dirtmask,";
        if (m_dirtmask) {
            auto image             = Image::create("/home/zhytou/tinyrenderer/asset/static/dirtmask.png");
            m_textures["dirtmask"] = std::make_shared<Texture>(image->getWidth(), image->getHeight(), GL_TEXTURE_2D, GL_RGBA32F, 1);
            m_textures["dirtmask"]->upload(image, 0);
        } else {
            m_textures["dirtmask"] = std::make_shared<Texture>(1, 1, GL_TEXTURE_2D, GL_RGBA32F, 1);
        }
        std::cout << "]\n";
    }

    // 6. Create samplers for corresponding slots
    {
        std::unordered_map<uint32_t, std::shared_ptr<Sampler>> samplers;
        samplers[0x00FF]     = std::make_shared<Sampler>(SamplerDesc{
            // slot 0~7 -> GL_TEXTURE0~GL_TEXTURE7 = material textures
            .minFilter = GL_LINEAR_MIPMAP_LINEAR,
            .magFilter = GL_LINEAR,
            .wrapS     = GL_REPEAT,
            .wrapT     = GL_REPEAT,
        });
        samplers[0x0F00]     = std::make_shared<Sampler>(SamplerDesc{
            // slot 8~11 -> GL_TEXTURE8~GL_TEXTURE11 = gbuffer textures
            .minFilter = GL_NEAREST,
            .magFilter = GL_NEAREST,
            .wrapS     = GL_CLAMP_TO_EDGE,
            .wrapT     = GL_CLAMP_TO_EDGE,
        });
        samplers[0xF000]     = std::make_shared<Sampler>(SamplerDesc{
            // slot 12~15 -> GL_TEXTURE12~GL_TEXTURE15 = shadow textures
            .minFilter   = GL_NEAREST,
            .magFilter   = GL_NEAREST,
            .wrapS       = GL_CLAMP_TO_BORDER,
            .wrapT       = GL_CLAMP_TO_BORDER,
            .borderColor = {1.0f, 1.0f, 1.0f, 1.0f},
        });
        samplers[0xFF0000]   = std::make_shared<Sampler>(SamplerDesc{
            // slot 16~23 -> GL_TEXTURE16~GL_TEXTURE23 = skybox/ibl textures
            .minFilter = GL_LINEAR,
            .magFilter = GL_LINEAR,
            .wrapS     = GL_CLAMP_TO_EDGE,
            .wrapT     = GL_CLAMP_TO_EDGE,
            .wrapR     = GL_CLAMP_TO_EDGE,
        });
        samplers[0xFF000000] = std::make_shared<Sampler>(SamplerDesc{
            // slot 24~31 -> GL_TEXTURE24~GL_TEXTURE31 = postprocess textures
            .minFilter = GL_LINEAR,
            .magFilter = GL_LINEAR,
            .wrapS     = GL_CLAMP_TO_EDGE,
            .wrapT     = GL_CLAMP_TO_EDGE,
        });
        for (auto& [name, slot] : m_texture2SlotIndexs) {
            if (slot >= 32 || slot < 0) {
                continue;
            }
            for (auto& [mask, sampler] : samplers) {
                if (mask & (1 << slot)) {
                    m_samplers[name] = sampler;
                    sampler->bind(slot);
                }
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
    StaticResource& instance = StaticResource::getInstance();
    auto cubeLayout          = instance.getLayout("cube");
    auto quadLayout          = instance.getLayout("quad");
    GLsizei cubeCount        = instance.getCounts("cube");
    GLsizei quadCount        = instance.getCounts("quad");

    // 1. Convert equirect skybox into cube map skybox if needed
    const auto& cubemap           = scene.getSkyboxCubeMap();
    m_textures["skybox.equirect"] = scene.getSkyboxEquirect();
    if (cubemap == nullptr && m_textures["skybox.equirect"] != nullptr) {
        m_states["equirect_to_cubemap"].apply();
        m_shaders["equirect_to_cubemap"]->use();
        for (GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X; face <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z; face++) {
            GLint index = face - GL_TEXTURE_CUBE_MAP_POSITIVE_X;
            m_shaders["equirect_to_cubemap"]->setUniformValue("uViewProjMatrix", instance.getCaptureMatrix(index));
            m_frames["skybox"]->attach(GL_COLOR_ATTACHMENT0, m_textures["skybox.cubemap"], 0, index);
            m_passes["equirect_to_cubemap"].begin(m_frames["skybox"]);
            draw(cubeLayout, {"skybox.equirect"}, cubeCount);
            m_passes["equirect_to_cubemap"].end();
        }
    } else {
        m_textures["skybox.cubemap"] = scene.getSkyboxCubeMap();
    }

    // 2. Precalculate environment map
    if (m_ibl) {
        // 2.1 Precalculate irradiance map
        {
            m_states["ibl_irradiance"].apply();
            m_shaders["ibl_irradiance"]->use();
            for (GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X; face <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z; face++) {
                GLint index = face - GL_TEXTURE_CUBE_MAP_POSITIVE_X;
                m_shaders["ibl_irradiance"]->setUniformValue("uViewProjMatrix", instance.getCaptureMatrix(index));
                m_frames["ibl_diffuse"]->attach(GL_COLOR_ATTACHMENT0, m_textures["ibl_diffuse"], 0, index);
                m_passes["ibl_irradiance"].begin(m_frames["ibl_diffuse"]);
                draw(cubeLayout, {"skybox.cubemap"}, cubeCount);
                m_passes["ibl_irradiance"].end();
            }
        }

        // 2.2 Precalculate prefiltered environment map
        {
            m_states["ibl_prefiltered"].apply();
            m_shaders["ibl_prefiltered"]->use();
            for (GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X; face <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z; face++) {
                GLint index       = face - GL_TEXTURE_CUBE_MAP_POSITIVE_X;
                GLsizei mipLevels = m_textures["ibl_specular"]->getMipLevels();
                m_shaders["ibl_prefiltered"]->setUniformValue("uViewProjMatrix", instance.getCaptureMatrix(index));

                for (GLsizei level = 0; level < mipLevels; level++) {
                    float roughness = static_cast<float>(level) / mipLevels;

                    m_shaders["ibl_prefiltered"]->setUniformValue("uRoughness", roughness);
                    m_frames["ibl_specular"]->attach(GL_COLOR_ATTACHMENT0, m_textures["ibl_specular"], level, index);

                    m_passes["ibl_prefiltered"].begin(m_frames["ibl_specular"]);
                    draw(cubeLayout, {"skybox.cubemap"}, cubeCount);
                    m_passes["ibl_prefiltered"].end();
                }
            }
        }

        // 2.3 Precalculate BRDF LUT
        {
            m_states["ibl_brdf_lut"].apply();
            m_shaders["ibl_brdf_lut"]->use();
            m_passes["ibl_brdf_lut"].begin(m_frames["ibl_brdf_lut"]);
            draw(quadLayout, {}, quadCount);
            m_passes["ibl_brdf_lut"].end();
        }
    } else {
        glm::vec4 color = {0.0f, 0.0f, 0.0f, 1.0f};
        m_textures["ibl_diffuse"]->clear(glm::value_ptr(color), GL_RGBA, GL_FLOAT, 0);
        for (GLint level = 0; level < m_textures["ibl_specular"]->getMipLevels(); level++) {
            m_textures["ibl_specular"]->clear(glm::value_ptr(color), GL_RGBA, GL_FLOAT, level);
        }
        m_textures["ibl_brdf_lut"]->clear(glm::value_ptr(color), GL_RGBA, GL_FLOAT, 0);
    }
}

void Renderer::update(const Scene& scene) {
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
            m_buffers["model"] = std::make_shared<UniformBuffer>(std::max(sizeof(ModelBlock) * modelBlocks.size(), 1ul));
        }
        m_buffers["model"]->upload(0, std::max(sizeof(ModelBlock) * modelBlocks.size(), 1ul), modelBlocks.data());
        m_buffers["model"]->bind(1);

        // 1.3 Light shader storage array
        std::vector<LightBlock> lightBlocks;
        scene.getLightBlocks(lightBlocks);
        if (m_buffers["light"] == nullptr) {
            m_buffers["light"] = std::make_shared<ShaderStorageBuffer>(std::max(sizeof(LightBlock) * lightBlocks.size(), 1ul));
        }
        m_buffers["light"]->upload(0, std::max(sizeof(LightBlock) * lightBlocks.size(), 1ul), lightBlocks.data());
        m_buffers["light"]->bind(0);
    }

    // 2. Generate render item queue(draw command)
    StaticResource& instance = StaticResource::getInstance();
    std::vector<RenderItem> opaqueQueue, transparentQueue;
    scene.getRenderQueue(opaqueQueue, true);
    scene.getRenderQueue(transparentQueue, false);

    // 3. Render shadow map and update the light shader storage buffer if needed
    if (m_shadow) {
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
        m_buffers["light"]->upload(0, std::max(sizeof(LightBlock) * lightBlocks.size(), 1ul), lightBlocks.data());
    } else {
        float depth = 1.0f;
        m_textures["shadow"]->clear(&depth, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    }
}

void Renderer::render(const Scene& scene) {
    StaticResource& instance = StaticResource::getInstance();
    std::vector<RenderItem> opaqueQueue, transparentQueue;
    scene.getRenderQueue(opaqueQueue, true);
    scene.getRenderQueue(transparentQueue, false);
    // std::cout << "OpaqueQueue size: " << opaqueQueue.size() << std::endl;
    // std::cout << "TransparentQueue size: " << transparentQueue.size() << std::endl;

    if (m_deferred) {
        {
            m_states["deferred_geometry"].apply();
            m_shaders["deferred_geometry"]->use();
            m_passes["deferred_geometry"].begin(m_frames["gbuffer"]);
            for (const auto& item : opaqueQueue) {
                m_buffers["model"]->bind(1, item.uoffset, sizeof(ModelBlock));
                draw(item, {"albedo", "normal", "mrao"});
            }
            m_passes["deferred_geometry"].end();
        }

        // ! WARNING: Copy depth buffer to screen framebuffer, otherwise depth test will fail for subsequent passes that bind screen framebuffer, since gbuffer's depth buffer is not shared with screen framebuffer.(e.g., skybox will fail if copy is commented) This is a workaround for the fact that OpenGL does not support framebuffer inheritance and subpasses like Vulkan, which allow multiple passes to share the same depth attachment without copying.
        m_frames["hdr_screen"]->copy(*m_frames["gbuffer"], GL_DEPTH_BUFFER_BIT);

        {
            GLsizei count = instance.getCounts("quad");
            auto& layout  = instance.getLayout("quad");

            m_states["deferred_shading"].apply();
            m_shaders["deferred_shading"]->use();
            m_shaders["deferred_shading"]->setUniformValue("uLightCount", (int)scene.getLights().size());
            m_passes["deferred_shading"].begin(m_frames["hdr_screen"]);
            draw(layout, {"gbuffer.albedo", "gbuffer.normal", "gbuffer.mrao", "shadow", "ibl_diffuse", "ibl_specular", "ibl_brdf_lut"}, count);
            m_passes["deferred_shading"].end();
        }
    } else {
        m_states["forward_opaque"].apply();
        m_shaders["forward_opaque"]->use();
        m_shaders["forward_opaque"]->setUniformValue("uLightCount", (int)scene.getLights().size());
        m_passes["forward_opaque"].begin(m_frames["hdr_screen"]);
        for (const auto& item : opaqueQueue) {
            m_buffers["model"]->bind(1, item.uoffset, sizeof(ModelBlock));
            draw(item, {"albedo", "normal", "mrao", "shadow", "ibl_diffuse", "ibl_specular", "ibl_brdf_lut"});
        }
        m_passes["forward_opaque"].end();
    }

    if (m_textures["skybox.cubemap"] != nullptr) {
        GLsizei count = instance.getCounts("cube");
        auto& layout  = instance.getLayout("cube");

        m_states["skybox"].apply();
        m_shaders["skybox"]->use();
        m_passes["skybox"].begin(m_frames["hdr_screen"]);
        draw(layout, {"skybox.cubemap"}, count);
        m_passes["skybox"].end();
    }

    if (m_bloom || m_lensflare) {
        GLsizei count = instance.getCounts("quad");
        auto& layout  = instance.getLayout("quad");

        {
            m_states["postprocess_highlight"].apply();
            m_shaders["postprocess_highlight"]->use();
            m_passes["postprocess_highlight"].begin(m_frames["highlight"]);
            draw(layout, {"hdr_screen.color"}, count);
            m_passes["postprocess_highlight"].end();
        }

        {
            m_frames["blur_down"]->attach(GL_COLOR_ATTACHMENT0, m_textures["blur_down"], 0);      // reset blur_down to mip level 0 before copying, as the previous frame's loop leaves it attached to the smallest mip level, while highlight only has one level, no adjustment is needed for it.
            m_frames["blur_down"]->copy(*m_frames["highlight"], GL_COLOR_BUFFER_BIT, GL_LINEAR);  // GL_LINEAR is used to handle bilinear interpolation since their dimensions may differ.
        }

        m_states["postprocess_kawase_down"].apply();
        m_shaders["postprocess_kawase_down"]->use();
        for (int level = 0; level + 1 < m_bloomMipLevels; level++) {
            int srcLevel        = level;
            int dstLevel        = level + 1;
            float srcTexelSizeX = 1.0f / (float)m_textures["blur_down"]->getWidth(srcLevel);
            float srcTexelSizeY = 1.0f / (float)m_textures["blur_down"]->getHeight(srcLevel);
            uint32_t width      = m_textures["blur_down"]->getWidth(dstLevel);
            uint32_t height     = m_textures["blur_down"]->getHeight(dstLevel);

            m_frames["blur_down"]->attach(GL_COLOR_ATTACHMENT0, m_textures["blur_down"], dstLevel);
            m_shaders["postprocess_kawase_down"]->setUniformValue("uSrcTexelSizeX", srcTexelSizeX);
            m_shaders["postprocess_kawase_down"]->setUniformValue("uSrcTexelSizeY", srcTexelSizeY);
            m_shaders["postprocess_kawase_down"]->setUniformValue("uSrcLevel", srcLevel);
            m_states["postprocess_kawase_down"].view(0, 0, width, height);
            m_passes["postprocess_kawase_down"].begin(m_frames["blur_down"]);
            m_textures["blur_down"]->clamp(srcLevel);  // avoid feedback loop
            draw(layout, {"blur_down"}, count);
            m_textures["blur_down"]->unclamp();
            m_passes["postprocess_kawase_down"].end();
        }

        {
            m_frames["blur_down"]->attach(GL_COLOR_ATTACHMENT0, m_textures["blur_down"], m_bloomMipLevels - 1);
            m_frames["blur_up"]->attach(GL_COLOR_ATTACHMENT0, m_textures["blur_up"], m_bloomMipLevels - 1);
            m_frames["blur_up"]->copy(*m_frames["blur_down"], GL_COLOR_BUFFER_BIT);  // both framebuffers are needed to be explicitly attached to the final mip level before copying
        }

        m_states["postprocess_kawase_up"].apply();
        m_shaders["postprocess_kawase_up"]->use();
        for (int level = m_bloomMipLevels - 1; level - 1 >= 0; level--) {
            int srcLevel        = level;
            int dstLevel        = level - 1;
            float srcTexelSizeX = 1.0f / (float)m_textures["blur_down"]->getWidth(srcLevel);
            float srcTexelSizeY = 1.0f / (float)m_textures["blur_down"]->getHeight(srcLevel);
            GLsizei dstWidth    = m_textures["blur_up"]->getWidth(dstLevel);
            GLsizei dstHeight   = m_textures["blur_up"]->getHeight(dstLevel);

            m_frames["blur_up"]->attach(GL_COLOR_ATTACHMENT0, m_textures["blur_up"], dstLevel);
            m_shaders["postprocess_kawase_up"]->setUniformValue("uSrcTexelSizeX", srcTexelSizeX);
            m_shaders["postprocess_kawase_up"]->setUniformValue("uSrcTexelSizeY", srcTexelSizeY);
            m_shaders["postprocess_kawase_up"]->setUniformValue("uSrcLevel", srcLevel);
            m_shaders["postprocess_kawase_up"]->setUniformValue("uDstLevel", dstLevel);
            m_states["postprocess_kawase_up"].view(0, 0, dstWidth, dstHeight);
            m_passes["postprocess_kawase_up"].begin(m_frames["blur_up"]);
            m_textures["blur_up"]->clamp(srcLevel);  // avoid feedback loop(if commented out, dstLevel will be read inadvertently, causing the blur radius to expand continuously and amplify endlessly across frames.)
            draw(layout, {"blur_up", "blur_down"}, count);
            m_textures["blur_up"]->unclamp();
            m_passes["postprocess_kawase_up"].end();
        }

        m_textures["bloom"] = m_textures["blur_up"];
    }

    if (m_lensflare) {
        GLsizei count = instance.getCounts("quad");
        auto& layout  = instance.getLayout("quad");

        {
            m_states["postprocess_lensflare"].apply();
            m_shaders["postprocess_lensflare"]->use();
            m_passes["postprocess_lensflare"].begin(m_frames["lensflare"]);
            draw(layout, {"bloom"}, count);  // use blurred highlight(namely bloom) as input to generate raw lensflare(ghost/halo/distortion).
            m_passes["postprocess_lensflare"].end();
        }

        {
            m_frames["blur_y"]->attach(GL_COLOR_ATTACHMENT0, m_textures["blur_y"], 0);
            m_frames["lensflare"]->attach(GL_COLOR_ATTACHMENT0, m_textures["lensflare"], 0);
            m_frames["blur_y"]->copy(*m_frames["lensflare"], GL_COLOR_BUFFER_BIT, GL_LINEAR);  // blur raw lensflare in y direction first to generate final lensflare effect.
        }

        {
            bool xFilter     = true;
            float texelSizeX = 1.0f / (float)m_frames["blur_y"]->getWidth();
            float texelSizeY = 1.0f / (float)m_frames["blur_y"]->getHeight();

            m_states["postprocess_gaussian_blur"].apply();
            m_shaders["postprocess_gaussian_blur"]->use();
            m_shaders["postprocess_gaussian_blur"]->setUniformValue("uTexelSizeX", texelSizeX);
            m_shaders["postprocess_gaussian_blur"]->setUniformValue("uTexelSizeY", texelSizeY);
            for (int i = 0; i < m_lensflareBlurTimes * 2; i++) {
                m_shaders["postprocess_gaussian_blur"]->setUniformValue("uXFilter", xFilter);
                if (xFilter) {
                    m_passes["postprocess_gaussian_blur"].begin(m_frames["blur_x"]);
                    draw(layout, {"blur_y"}, count);
                } else {
                    m_passes["postprocess_gaussian_blur"].begin(m_frames["blur_y"]);
                    draw(layout, {"blur_x"}, count);
                }
                m_passes["postprocess_gaussian_blur"].end();
                xFilter = !xFilter;
            }
        }

        {
            m_frames["blur_y"]->attach(GL_COLOR_ATTACHMENT0, m_textures["blur_y"], 0);
            m_frames["lensflare"]->attach(GL_COLOR_ATTACHMENT0, m_textures["lensflare"], 0);
            m_frames["lensflare"]->copy(*m_frames["blur_y"], GL_COLOR_BUFFER_BIT, GL_LINEAR);  // replace the lensflare texture with the blurred one.
        }
    }

    // TODO: screen space ambient occlusion
    if (m_ssao) {
    }

    // TODO: temporal anti aliasing
    if (m_taa) {
    }

    {
        GLsizei count = instance.getCounts("quad");
        auto& layout  = instance.getLayout("quad");

        m_states["postprocess_final"].apply();
        m_states["postprocess_final"].view(0, 0, m_width, m_height);
        m_shaders["postprocess_final"]->use();
        m_passes["postprocess_final"].begin(m_frames["screen"]);
        draw(layout, {"hdr_screen.color", "highlight", "blur_up", "blur_down", "lensflare", "dirtmask"}, count);
        m_passes["postprocess_final"].end();
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

    for (auto name : textures) {
        m_textures[name]->unbind(m_texture2SlotIndexs[name]);
    }

    m_drawCall++;
    return;
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
    for (auto name : textures) {
        m_textures[name]->unbind(m_texture2SlotIndexs[name]);
    }

    m_drawCall++;
    return;
}

}  // namespace tinyrenderer

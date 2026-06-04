// RenderState.h
#pragma once
#include <glad/glad.h>

#include <memory>

#include "shader.hpp"

namespace tinyrenderer {

/**
 * @brief Data block containing the fixed-function states of the graphics pipeline.
 * @note Encapsulates discrete OpenGL states (rasterization, blending, depth/stencil)
 * into a single data block. This data-driven approach prevents state leakage
 * and mimics modern Pipeline State Objects (PSO) found in Vulkan and DX12.
 */
struct PipelineState {
    // rasterization config
    GLboolean cullEnable = false;
    GLenum polygonMode   = GL_FILL;
    GLenum cullMode      = GL_BACK;
    GLenum frontFace     = GL_CCW;

    // color blend config
    GLboolean blendEnable = false;
    GLenum srcBlend       = GL_SRC_ALPHA;
    GLenum dstBlend       = GL_ONE_MINUS_SRC_ALPHA;

    // depth test and stencil test config
    GLboolean depthTestEnable   = true;
    GLboolean depthWriteEnable  = true;
    GLenum depthFunc            = GL_LESS;
    GLboolean stencilTestEnable = false;

    inline void apply();
};

inline void PipelineState::apply() {
    if (depthTestEnable) {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(depthFunc);
    } else {
        glDisable(GL_DEPTH_TEST);
    }
    glDepthMask(depthWriteEnable ? GL_TRUE : GL_FALSE);

    if (cullEnable) {
        glEnable(GL_CULL_FACE);
        glCullFace(cullMode);
    } else {
        glDisable(GL_CULL_FACE);
    }
    glPolygonMode(GL_FRONT_AND_BACK, polygonMode);
    glFrontFace(frontFace);

    if (blendEnable) {
        glEnable(GL_BLEND);
        glBlendFunc(srcBlend, dstBlend);
    } else {
        glDisable(GL_BLEND);
    }
}

}  // namespace tinyrenderer
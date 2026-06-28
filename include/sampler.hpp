#pragma once

#include <glad/glad.h>

namespace tinyglrenderer {

struct SamplerDesc {
    // Texture Filtering
    // | Macro                     | Behavior                  | Available For       | Typical Usage                     |
    // |---------------------------|---------------------------|---------------------|-----------------------------------|
    // | GL_NEAREST                | Nearest pixel (no blend)  | Min / Mag           | Shadow map, pixel art, UI         |
    // | GL_LINEAR                 | Bilinear blend (smooth)   | Min / Mag           | Common PBR texture, general image |
    // | GL_NEAREST_MIPMAP_NEAREST | Nearest + nearest mip     | Min only            | Fast low-quality mipmapping       |
    // | GL_LINEAR_MIPMAP_NEAREST  | Linear + nearest mip      | Min only            | Balanced performance & quality    |
    // | GL_NEAREST_MIPMAP_LINEAR  | Nearest + linear mip blend | Min only           | Smooth mip transition             |
    // | GL_LINEAR_MIPMAP_LINEAR   | Trilinear filtering       | Min only            | Highest quality, terrain/landscape|
    // Note: magFilter only supports GL_NEAREST / GL_LINEAR (no mipmap variants)
    GLenum minFilter = GL_LINEAR_MIPMAP_LINEAR; // minification filter (texture scaled down)
    GLenum magFilter = GL_LINEAR;               // magnification filter (texture scaled up), NO mipmap modes allowed

    // Texture Wrapping (UV addressing mode)
    // | Macro               | Behavior when UV out of [0, 1] | Extra Config Required | Typical Usage                     |
    // |---------------------|--------------------------------|-----------------------|-----------------------------------|
    // | GL_REPEAT           | Tile texture repeatedly        | No                    | Floor, wall, seamless tile texture|
    // | GL_MIRRORED_REPEAT  | Mirror then tile texture       | No                    | Natural texture, reduce tile seam |
    // | GL_CLAMP_TO_EDGE    | Extend texture edge pixels     | No                    | Model texture, UI, skybox         |
    // | GL_CLAMP_TO_BORDER  | Use custom border color        | Yes (borderColor)     | Shadow depth map, special effect  |
    GLenum wrapS = GL_REPEAT; // wrap mode for S coordinate (U / horizontal axis)
    GLenum wrapT = GL_REPEAT; // wrap mode for T coordinate (V / vertical axis)
    GLenum wrapR = GL_REPEAT; // wrap mode for R coordinate (3D texture / cubemap only)

    // Border Color
    float borderColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f }; // border color for GL_CLAMP_TO_BORDER mode

    // Level of Detail (LOD)
    GLfloat lodBias = 0.0f; // LOD bias for mipmap sampling
    GLfloat minLod  = -1000.0f;
    GLfloat maxLod  = 1000.0f;

    /// Depth texture
    GLenum compareMode = GL_NONE; // depth compare mode (for shadow depth texture)
    GLenum compareFunc = GL_LESS; // depth compare function (valid when compareMode != GL_NONE)
};

/**
 * @brief Encapsulates a GPU sampler state object (OpenGL Sampler Object).
 * @note Decouples texture sampling states (filtering, wrapping, mipmapping) from the underlying
 * texture data storage. Eliminates the state-switching overhead of traditional texture-bound
 * parameters by enabling independent, modular binding to texture units.
 */
class Sampler {
   public:
    Sampler();
    Sampler(const SamplerDesc& desc);
    Sampler(const Sampler&)            = delete;
    Sampler& operator=(const Sampler&) = delete;
    Sampler(Sampler&& other);
    Sampler& operator=(Sampler&& other);
    ~Sampler();

    // Bind sampler to a specific sampler slot, same as the texture binding index
    // @param slot The sampler slot index
    void bind(GLuint slot) const;

    // Set sampler parameters
    // @param desc The sampler description
    void set(const SamplerDesc& desc);

   private:
    //! WARNING: m_id = 0 is OpenGL default sampler (driver-managed, immutable, useing texture internal parameters).
    GLuint m_id = 0;
};

} // namespace tinyglrenderer
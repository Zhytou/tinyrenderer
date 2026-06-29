#pragma once

#include <glad/glad.h>

#include <filesystem>
#include <glm/glm.hpp>
#include <memory>
#include <vector>

#include "image.hpp"

namespace tinyglrenderer {

/**
 * @brief Encapsulates a GPU texture resource managing layout and multi-dimensional image storage.
 * @note Acts as a dedicated memory container on the GPU (supporting 2D textures, mipmaps, and Cubemaps).
 * Defines the underlying physical storage data layout, including internal channel formats and dimensional boundaries, while leaving texel fetching/filtering logic to decoupled Sampler objects.
 */
class Texture {
   public:
    Texture(GLsizei size, GLenum target, GLenum internalFormat, GLsizei mipLevels);
    Texture(GLsizei width, GLsizei height, GLenum target, GLenum internalFormat, GLsizei mipLevels);
    Texture(GLsizei width, GLsizei height, GLsizei depth, GLenum target, GLenum internalFormat, GLsizei mipLevels);
    Texture(const Texture&)            = delete;
    Texture& operator=(const Texture&) = delete;
    Texture(Texture&& other);
    Texture& operator=(Texture&& other);
    ~Texture();

    GLsizei getWidth(GLint level) const;
    GLsizei getHeight(GLint level) const;
    GLsizei getDepth(GLint level) const;
    GLuint getID() const { return m_id; }
    GLenum getTarget() const { return m_target; }
    GLenum getInternalFormat() const { return m_internalFormat; }
    GLsizei getMipLevels() const { return m_mipLevels; }

    // Bind texture to a specific texture slot, namely the glsl binding index
    // @param slot The texture slot to bind to.
    void bind(GLuint slot) const;
    // Unbind texture from a specific texture slot.
    // @param slot The texture slot to unbind from.
    void unbind(GLuint slot) const;
    // Unbind texture from all slots.
    static void unbind();

    // Clear the entire texture image storage to a constant value.
    // @param value Pointer to a single texel data containing the clear value. Pass 'nullptr' to clear the entire texture to black / zero natively.
    // @param format The pixel format of the incoming single texel(e.g., GL_RGBA, GL_RED).
    // @param type The data type of the incoming single texel(e.g., GL_UNSIGNED_BYTE, GL_FLOAT).
    // @param level The target mipmap level to clear.
    void clear(const void* value, GLenum format, GLenum type, GLint level = 0);

    // Upload texutre data to existed 2d texture object
    // @note The input data size must match up with the texture object.
    // @param img The image data to upload.
    // @param level The mip level to upload.
    void upload(const std::shared_ptr<Image>& img, GLint level = 0);

    // Upload texutre data to existed cubemap texture object
    // @note The input image data size must match up with the texture object.
    // @param img   The image data to upload.
    // @param pos   The cubemap face to upload to.
    //             0: GL_TEXTURE_CUBE_MAP_POSITIVE_X (Right)
    //             1: GL_TEXTURE_CUBE_MAP_NEGATIVE_X (Left)
    //             2: GL_TEXTURE_CUBE_MAP_POSITIVE_Y (Top)
    //             3: GL_TEXTURE_CUBE_MAP_NEGATIVE_Y (Bottom)
    //             4: GL_TEXTURE_CUBE_MAP_POSITIVE_Z (Back)
    //             5: GL_TEXTURE_CUBE_MAP_NEGATIVE_Z (Front)
    // @param level The mip level to upload.
    void upload(const std::shared_ptr<Image>& img, GLint pos, GLint level);

    // Copy the texture data from one texture object to another.
    // @param src The source texture object to copy from.
    // @param srcLevel  The source mip level to copy from.
    // @param srcX      The source X coordinate to copy from.
    // @param srcY      The source Y coordinate to copy from.
    // @param srcZ      The source Z coordinate to copy from.
    // @param dstLevel  The destination mip level to copy to.
    // @param dstX      The destination X coordinate to copy to.
    // @param dstY      The destination Y coordinate to copy to.
    // @param dstZ      The destination Z coordinate to copy to.
    void copy(const Texture& src, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ);

    // Clamp the hardware visibility of the texture's mipmap chain to a single discrete level.
    // @param level The mip level to clamp to
    // @note Crucial for Multi-pass Post-processing (e.g., Bloom Pyramids) to prevent OpenGL Rendering Feedback Loops (Spec 9.3). By physically forcing GL_TEXTURE_BASE_LEVEL and GL_TEXTURE_MAX_LEVEL to the same discrete integer, the GPU driver isolates this specific mip-slice, allowing other slices of the SAME texture to be safely attached as FBO targets without triggering undefined data race/corruption.
    // @warning Unlike GL_SAMPLER_MIN/MAX_LOD which are continuous float parameters managed by Sampler Objects, these parameters belong to the core Texture Object and alter its global memory-view bounds. Ensure to reset or manage wisely if this texture is reused in general sampling.
    void clamp(GLint level);
    // Reset the hardware visibility of the texture's mipmap chain to the default state.
    void unclamp();

    // Generate mipmaps for the texture object.
    void generate();

   private:
    GLsizei m_width         = 0;
    GLsizei m_height        = 0;
    GLsizei m_depth         = 0;
    GLuint m_id             = 0;
    GLenum m_target         = GL_TEXTURE_2D; // texture target indicates the target to bind and upload texture data to, and also how the texture storage is organized in GPU memory (e.g., 2D array for GL_TEXTURE_2D, or 6-face cube for GL_TEXTURE_CUBE_MAP)
    GLenum m_internalFormat = GL_RGBA8;      // texture gpu format indicates both the channel ORDER and the data TYPE (e.g., GL_RGBA8 for 8-bit RGBA format, GL_RGB16F for 16-bit float RGB format, GL_R32F for 32-bit float R format, etc.)
    GLsizei m_mipLevels     = 1;
};

} // namespace tinyglrenderer
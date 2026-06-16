#pragma once

#include <glad/glad.h>

#include <filesystem>
#include <glm/glm.hpp>
#include <memory>
#include <vector>

#include "image.hpp"

namespace tinyrenderer {

/**
 * @brief Encapsulates a GPU texture resource managing layout and multi-dimensional image storage.
 * @note Acts as a dedicated memory container on the GPU (supporting 2D textures, mipmaps, and Cubemaps).
 * Defines the underlying physical storage data layout, including internal channel formats and dimensional boundaries, while leaving texel fetching/filtering logic to decoupled Sampler objects.
 */
class Texture {
   public:
    Texture(uint32_t width, uint32_t height, GLenum type, GLenum internalFormat, GLuint mipLevel = 1);
    Texture(const Texture&)            = delete;
    Texture& operator=(const Texture&) = delete;
    Texture(Texture&& other);
    Texture& operator=(Texture&& other);
    ~Texture();

    GLuint getId() const { return m_id; }
    GLenum getType() const { return m_type; }
    uint32_t getWidth() const { return m_width; }
    uint32_t getHeight() const { return m_height; }
    GLenum getInternalFormat() const { return m_internalFormat; }

    // Bind texture to a specific texture slot, namely the glsl binding index
    // @param slot The texture slot to bind to.
    void bind(uint32_t slot) const;

    // Create a 1*1 texture from a vec4 value
    // @param value The value to create the texture from.
    // @param internalFormat The internal format of the texture.
    // @return The created texture.
    static std::shared_ptr<Texture> create(const glm::vec4& value, GLenum internalFormat);

    // Create a 2d texture from a single image file, and upload the image data to GPU memory
    // @param path The image file path to create the texture from.
    // @param internalFormat The internal format of the texture.
    // @param desiredChannels The desired channels of the texture.
    // @return The created texture.
    static std::shared_ptr<Texture> create(const std::filesystem::path& path, GLenum internalFormat, int desiredChannels = 0);

    // Create a 2d or cubemap texture from multiple image files, and upload the image data to GPU memory
    // @param paths The image file paths to create the texture from.
    // @param type The texture type.
    // @param internalFormat The internal format of the texture.
    // @param desiredChannels The desired channels of the texture.
    // @return The created texture.
    static std::shared_ptr<Texture> create(const std::vector<std::filesystem::path>& paths, GLenum type, GLenum internalFormat, int desiredChannels = 0);

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
    // @note The input data size must match up with the texture object.
    // @param img   The image data to upload.
    // @param pos   The cubemap face to upload to.
    //             0: GL_TEXTURE_CUBE_MAP_POSITIVE_X (Right)
    //             1: GL_TEXTURE_CUBE_MAP_NEGATIVE_X (Left)
    //             2: GL_TEXTURE_CUBE_MAP_POSITIVE_Y (Top)
    //             3: GL_TEXTURE_CUBE_MAP_NEGATIVE_Y (Bottom)
    //             4: GL_TEXTURE_CUBE_MAP_POSITIVE_Z (Back)
    //             5: GL_TEXTURE_CUBE_MAP_NEGATIVE_Z (Front)
    // @param level The mip level to upload.
    void upload(const std::shared_ptr<Image>& img, GLint pos, GLint level = 0);

   private:
    uint32_t m_width        = 0;
    uint32_t m_height       = 0;
    GLuint m_id             = 0;
    GLenum m_type           = GL_TEXTURE_2D;  // texture type indicates the target to bind and upload texture data to, and also how the texture storage is organized in GPU memory (e.g., 2D array for GL_TEXTURE_2D, or 6-face cube for GL_TEXTURE_CUBE_MAP)
    GLenum m_internalFormat = GL_RGBA8;       // texture gpu format indicates both the channel ORDER and the data TYPE (e.g., GL_RGBA8 for 8-bit RGBA format, GL_RGB16F for 16-bit float RGB format, GL_R32F for 32-bit float R format, etc.)
    GLuint m_mipLevels      = 1;
};

}  // namespace tinyrenderer
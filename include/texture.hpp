#pragma once

#include <glad/glad.h>

#include <filesystem>
#include <glm/glm.hpp>
#include <memory>
#include <vector>

#include "image.hpp"

namespace tinyrenderer {

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
    void bind(uint32_t unit) const;

    // Create a 2d texture from a single image file, and upload the image data to GPU memory
    static std::shared_ptr<Texture> create(const std::filesystem::path& path, GLenum internalFormat, int desiredChannels = 0);

    // Create a 2d or cubemap texture from multiple image files, and upload the image data to GPU memory
    static std::shared_ptr<Texture> create(const std::initializer_list<std::filesystem::path>& paths, GLenum type, GLenum internalFormat, int desiredChannels = 0);

    // Upload texutre data to existed 2d texture handle
    void upload(const std::shared_ptr<Image>& img, uint32_t level = 0);

    // Upload texutre data to existed cubemap texture handle
    // void upload(const std::shared_ptr<Image>& img, GLenum pos);

   private:
    uint32_t m_width        = 0;
    uint32_t m_height       = 0;
    GLuint m_id             = 0;
    GLenum m_type           = GL_TEXTURE_2D;  // texture type
    GLenum m_internalFormat = GL_RGBA8;       //! NOTE: texture gpu format indicates both the channel ORDER and the data TYPE
    GLuint m_mipLevels      = 1;
};

}  // namespace tinyrenderer
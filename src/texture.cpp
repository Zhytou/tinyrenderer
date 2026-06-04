#include "texture.hpp"

#include <iostream>

namespace tinyrenderer {

namespace fs = std::filesystem;

Texture::Texture(uint32_t width, uint32_t height, GLenum type, GLenum internalFormat, GLuint mipLevel) : m_width(width), m_height(height), m_type(type), m_internalFormat(internalFormat), m_mipLevels(mipLevel) {
    // Create texture handle
    //
    // ┌──────────────────────────────────────────────────────────────────────────────────────────────┐
    // │                         glGenTextures vs glCreateTextures                                    │
    // ├───────────────────────────────────────────────┬──────────────────────────────────────────────┤
    // │              glGenTextures                    │              glCreateTextures                │
    // ├───────────────────────────────────────────────┼──────────────────────────────────────────────┤
    // │ • Generates UNINITIALIZED texture IDs         │ • Creates and INITIALIZES texture objects    │
    // │ • Requires separate glBindTexture call        │ • No bind needed (DSA style)                  │
    // │ • Must specify target at bind time            │ • Target specified at creation               │
    // │ • Changes global texture binding state        │ • Direct state access, no side effects       │
    // │ • Must call glBindTexture before config       │ • Configure directly via glTexture... APIs   │
    // │ • Traditional API (OpenGL 1.0+)               │ • Modern API (OpenGL 4.5+ / ARB_dsa)          │
    // │ • More verbose (gen + bind + config)          │ • Concise (create + direct config)           │
    // │ • Error prone (forget to bind)                │ • Safer (no binding state to track)          │
    // └───────────────────────────────────────────────┴──────────────────────────────────────────────┘
    //
    // Key insight:  glGenTextures  = "allocate ID only" (need bind + target to use)
    //               glCreateTextures = "allocate + initialize + target" (ready to configure)
    //
    // Example:
    //   Legacy: glGenTextures(1, &tex); glBindTexture(GL_TEXTURE_2D, tex); glTexImage2D(...);
    //   Modern: glCreateTextures(GL_TEXTURE_2D, 1, &tex); glTextureStorage2D(tex, ...);
    //
    glCreateTextures(m_type, 1, &m_id);

    // Allocate memory for texture storage
    //
    // ┌─────────────────────────────────────────────────────────────────────────────────┐
    // │                    glTexImage2D vs glTextureStorage2D                           │
    // ├──────────────────────────────┬──────────────────────────────────────────────────┤
    // │      glTexImage2D (Legacy)   │         glTextureStorage2D (Modern)              │
    // ├──────────────────────────────┼──────────────────────────────────────────────────┤
    // │ • Immutable AFTER upload     │ • COMPLETELY immutable (can't change format/size)│
    // │ • Can re-allocate anytime     │ • Must be allocated ONCE at creation             │
    // │ • Requires binding target     │ • Direct state access (no bind needed)           │
    // │ • Driver does more checking   │ • Driver can optimize better                    │
    // │ • Older API (< OpenGL 4.2)    │ • Modern API (OpenGL 4.2+ / DSA)                 │
    // └──────────────────────────────┴──────────────────────────────────────────────────┘
    //
    // Key insight: glTextureStorage2D = "allocate once, use forever" (best practice)
    //              glTexImage2D       = "flexible but slower" (legacy path)
    glTextureStorage2D(m_id, m_mipLevels, m_internalFormat, m_width, m_height);

    // Set sampler parameters
    glTextureParameteri(m_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(m_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(m_id, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(m_id, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

Texture::Texture(Texture&& other) {
    if (m_id) {
        glDeleteTextures(1, &m_id);
    }
    m_id             = other.m_id;
    m_type           = other.m_type;
    m_width          = other.m_width;
    m_height         = other.m_height;
    m_internalFormat = other.m_internalFormat;
    other.m_id       = 0;
}

Texture& Texture::operator=(Texture&& other) {
    if (m_id) {
        glDeleteTextures(1, &m_id);
    }
    m_id             = other.m_id;
    m_type           = other.m_type;
    m_width          = other.m_width;
    m_height         = other.m_height;
    m_internalFormat = other.m_internalFormat;
    other.m_id       = 0;
    return *this;
}

Texture ::~Texture() {
    if (m_id) glDeleteTextures(1, &m_id);
}

void Texture::bind(uint32_t unit) const {
    // Bind texture to a specific texture slot, namely the glsl binding index
    //
    // ┌─────────────────────────────────────────────────────────────────────────────────┐
    // │              glActiveTexture + glBindTexture  vs  glBindTextureUnit             │
    // ├───────────────────────────────────────────────┬─────────────────────────────────┤
    // │     Legacy: glActiveTexture + glBindTexture   │   Modern: glBindTextureUnit     │
    // ├───────────────────────────────────────────────┼─────────────────────────────────┤
    // │ • Two function calls                          │ • Single function call          │
    // │ • Changes global "active" state               │ • Direct, stateless binding     │
    // │ • Error-prone (forget glActiveTexture)        │ • Explicit unit specification   │
    // │ • Works with all OpenGL versions              │ • Requires OpenGL 4.5+ / DSA    │
    // │ • Must specify texture target (e.g., GL_TEXTURE_2D) │ • Target inferred from texture object │
    // │ • Slower (state validation overhead)          │ • Faster (direct state access)  │
    // └───────────────────────────────────────────────┴─────────────────────────────────┘
    glBindTextureUnit(unit, m_id);
}

std::shared_ptr<Texture> Texture::create(const fs::path& path, GLenum internalFormat, int desiredChannels) {
    std::cout << "Creating texture from image [" << path << "]\n";

    std::shared_ptr<Image> image     = Image::create(path, desiredChannels);
    std::shared_ptr<Texture> texture = nullptr;
    if (image) {
        texture = std::make_shared<Texture>(image->getWidth(), image->getHeight(), GL_TEXTURE_2D, internalFormat);
        texture->upload(image);
    }

    return texture;
}

std::shared_ptr<Texture> Texture::create(const std::initializer_list<fs::path>& paths, GLenum type, GLenum internalFormat, int desiredChannels) {
    std::vector<std::shared_ptr<Image>> images;

    std::cout << "Creating texture from images [";
    for (auto path : paths) {
        std::cout << path << ' ';
        auto image = Image::create(path, desiredChannels);
        images.push_back(image);
    }
    std::cout << "]\n";

    std::shared_ptr<Texture> texture = nullptr;
    if (type == GL_TEXTURE_2D) {
        std::shared_ptr<Image> mimage = Image::merge(images, 3);
        if (mimage) {
            texture = std::make_shared<Texture>(mimage->getWidth(), mimage->getHeight(), type, internalFormat);
            texture->upload(mimage);
        }
    }
    return texture;
}

void Texture::upload(const std::shared_ptr<Image>& img, uint32_t level) {
    std::cout << "Uploading texture data from image [" << img->getWidth() << ' ' << img->getHeight() << ' ' << img->getChannels() << ' ' << std::showbase << std::hex << img->getFormat() << ' ' << img->getDataType() << std::dec << "] to level" << level << " GPU memory\n";
    // Upload texture data to GPU memory
    //
    // ┌──────────────────────────────────────────────────────────────────────────────────────────┐
    // │                           glTexImage2D vs glTexSubImage2D                                 │
    // ├─────────────────────────────────────────────┬────────────────────────────────────────────┤
    // │              glTexImage2D                   │              glTexSubImage2D                │
    // ├─────────────────────────────────────────────┼────────────────────────────────────────────┤
    // │ • Allocates NEW texture storage             │ • Uploads data to EXISTING storage         │
    // │ • Can change format/size (re-allocate)      │ • Cannot change format/size                │
    // │ • Resets texture to uninitialized state     │ • Preserves other mip levels / regions     │
    // │ • Must specify full width/height            │ • Can update a sub-rectangle (x, y, w, h)  │
    // │ • More expensive (may reallocate GPU memory)│ • Cheap (just data transfer)               │
    // │ • Used for INITIAL allocation               │ • Used for UPDATE (streaming, dynamic tex) │
    // └─────────────────────────────────────────────┴────────────────────────────────────────────┘
    //
    // Key insight:  glTexImage2D  = "allocate + upload" (once at texture creation)
    //               glTexSubImage2D = "upload only" (update existing texture, e.g., video, dynamic UI)
    glTextureSubImage2D(m_id, level, 0, 0, m_width, m_height, img->getFormat(), img->getDataType(), img->getData());
}

}  // namespace tinyrenderer
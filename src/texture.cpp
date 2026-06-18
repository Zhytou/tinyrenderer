#include "texture.hpp"

#include <format>
#include <iostream>
#include <stdexcept>

#include "utils.hpp"

namespace tinyrenderer {

namespace fs = std::filesystem;

Texture::Texture(uint32_t size, GLenum type, GLenum internalFormat, GLsizei mipLevel) : m_type(type), m_internalFormat(internalFormat), m_mipLevel(mipLevel) {
    if (size == 0 || mipLevel > 10 || size < (1 << mipLevel)) {
        throw std::runtime_error(std::format("Texture::Texture: Invalid Texture size {} for mip level {}", size, mipLevel));
    }

    glCreateTextures(m_type, 1, &m_id);

    if (m_type == GL_TEXTURE_1D) {
        glTextureStorage1D(m_id, m_mipLevel, m_internalFormat, size);
    } else {
        throw std::runtime_error("Texture::Texture: Texture type is not 1D");
    }
}

Texture::Texture(uint32_t width, uint32_t height, GLenum type, GLenum internalFormat, GLsizei mipLevel) : m_width(width), m_height(height), m_type(type), m_internalFormat(internalFormat), m_mipLevel(mipLevel) {
    if (width == 0 || height == 0 || mipLevel > 10 || width < (1 << mipLevel) || height < (1 << mipLevel)) {
        throw std::runtime_error(std::format("Texture::Texture: Invalid Texture size {}x{} for mip level {}", width, height, mipLevel));
    }

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
    if (m_type == GL_TEXTURE_2D || m_type == GL_TEXTURE_CUBE_MAP) {
        glTextureStorage2D(m_id, m_mipLevel, m_internalFormat, m_width, m_height);
    } else {
        throw std::runtime_error("Texture::Texture: Texture type is not 2D or cube map");
    }
}

Texture::Texture(uint32_t width, uint32_t height, uint32_t depth, GLenum type, GLenum internalFormat, GLsizei mipLevel) : m_type(type), m_width(width), m_height(height), m_depth(depth), m_mipLevel(mipLevel), m_internalFormat(internalFormat) {
    if (width == 0 || height == 0 || depth == 0 || mipLevel > 10 || width < (1 << mipLevel) || height < (1 << mipLevel)) {
        throw std::runtime_error(std::format("Texture::Texture: Invalid Texture size {}x{}x{} for mip level {}", width, height, depth, mipLevel));
    }

    glCreateTextures(m_type, 1, &m_id);

    if (m_type == GL_TEXTURE_3D) {
        glTextureStorage3D(m_id, m_mipLevel, m_internalFormat, m_width, m_height, m_depth);
    } else {
        throw std::runtime_error("Texture::Texture: Texture type is not 3D");
    }
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
    if (m_id) {
        glDeleteTextures(1, &m_id);
    }
}

void Texture::bind(uint32_t slot) const {
    // Bind texture to a specific texture slot, namely the glsl binding index
    //
    // ┌─────────────────────────────────────────────────────────────────────────────────┐
    // │              glActiveTexture + glBindTexture  vs  glBindTextureUnit             │
    // ├───────────────────────────────────────────────┬─────────────────────────────────┤
    // │     Legacy: glActiveTexture + glBindTexture   │   Modern: glBindTextureUnit     │
    // ├───────────────────────────────────────────────┼─────────────────────────────────┤
    // │ • Two function calls                          │ • Single function call          │
    // │ • Changes global "active" state               │ • Direct, stateless binding     │
    // │ • Error-prone (forget glActiveTexture)        │ • Explicit slot specification   │
    // │ • Works with all OpenGL versions              │ • Requires OpenGL 4.5+ / DSA    │
    // │ • Must specify texture target (e.g., GL_TEXTURE_2D) │ • Target inferred from texture object │
    // │ • Slower (state validation overhead)          │ • Faster (direct state access)  │
    // └───────────────────────────────────────────────┴─────────────────────────────────┘
    glBindTextureUnit(slot, m_id);
}

void Texture::clear(const void* value, GLenum format, GLenum type, GLint level) {
    if (m_id == 0) {
        return;
    }

    glClearTexSubImage(m_id, level, 0, 0, 0, m_width, m_height, 1, format, type, value);
}

void Texture::upload(const std::shared_ptr<Image>& img, GLint level) {
    // std::cout << "Uploading texture data from image [" << img->getWidth() << ' ' << img->getHeight() << ' ' << img->getChannels() << ' ' << std::showbase << std::hex << img->getFormat() << ' ' << img->getDataType() << std::dec << "] to level" << level << " GPU memory\n";

    // Check if image data is valid
    if (img == nullptr || img->getData() == nullptr) {
        throw std::runtime_error("Texture::upload: image data is null");
    }
    if (level < 0 || level >= m_mipLevel) {
        throw std::runtime_error(std::format("Texture::upload: mip level {} out of range [0 - {}]", level, m_mipLevel));
    }
    uint32_t width  = img->getWidth();
    uint32_t height = img->getHeight();
    if (width > m_width / (1 << level) || height > m_height / (1 << level)) {
        throw std::runtime_error(std::format("Texture::upload: image size {}x{} does not match texture size {}x{} at level {}", width, height, m_width / (1 << level), m_height / (1 << level), level));
    }

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
    glTextureSubImage2D(m_id, level, 0, 0, width, height, img->getFormat(), img->getDataType(), img->getData());

    // Generate mipmaps if required
    if (level == 0 && m_mipLevel > 1) {
        glGenerateMipmap(m_id);
    }
}

void Texture::upload(const std::shared_ptr<Image>& img, GLint pos, GLint level) {
    // std::cout << "Uploading texture data from image [" << img->getWidth() << ' ' << img->getHeight() << ' ' << img->getChannels() << ' ' << std::showbase << std::hex << img->getFormat() << ' ' << img->getDataType() << std::dec << "] to level" << level << " GPU memory\n";

    // Check if image data is valid
    if (img == nullptr || img->getData() == nullptr) {
        throw std::runtime_error("Texture::upload: image data is null");
    }
    if (level < 0 || level >= m_mipLevel) {
        throw std::runtime_error(std::format("Texture::upload: mip level {} out of range [0 - {}]", level, m_mipLevel));
    }
    uint32_t width  = img->getWidth();
    uint32_t height = img->getHeight();
    if (width > m_width / (1 << level) || height > m_height / (1 << level)) {
        throw std::runtime_error(std::format("Texture::upload: image size {}x{} does not match texture size {}x{} at level {}", width, height, m_width / (1 << level), m_height / (1 << level), level));
    }

    // Upload texture data to GPU memory
    glTextureSubImage3D(m_id, level, 0, 0, pos, width, height, 1, img->getFormat(), img->getDataType(), img->getData());

    // Generate mipmaps if required
    if (level == 0 && m_mipLevel > 1) {
        glGenerateMipmap(m_id);
    }
}

}  // namespace tinyrenderer
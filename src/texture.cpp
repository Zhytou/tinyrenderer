#include "texture.hpp"

#include <format>
#include <iostream>
#include <stdexcept>

#include "utils.hpp"

namespace tinyrenderer {

namespace fs = std::filesystem;

Texture::Texture(uint32_t size, GLenum type, GLenum internalFormat, GLsizei mipLevels) : m_type(type), m_internalFormat(internalFormat), m_mipLevels(mipLevels) {
    if (size == 0 || mipLevels > 10 || size < (1 << mipLevels)) {
        throw std::runtime_error(std::format("Texture::Texture: Invalid Texture size {} for mip level {}", size, mipLevels));
    }

    glCreateTextures(m_type, 1, &m_id);

    if (m_type == GL_TEXTURE_1D) {
        glTextureStorage1D(m_id, m_mipLevels, m_internalFormat, size);
    } else {
        throw std::runtime_error("Texture::Texture: Texture type is not 1D");
    }
}

Texture::Texture(uint32_t width, uint32_t height, GLenum type, GLenum internalFormat, GLsizei mipLevels) : m_width(width), m_height(height), m_type(type), m_internalFormat(internalFormat), m_mipLevels(mipLevels) {
    if (width == 0 || height == 0 || mipLevels > 10 || width < (1 << mipLevels) || height < (1 << mipLevels)) {
        throw std::runtime_error(std::format("Texture::Texture: Invalid Texture size {}x{} for mip level {}", width, height, mipLevels));
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
        glTextureStorage2D(m_id, m_mipLevels, m_internalFormat, m_width, m_height);
    } else {
        throw std::runtime_error("Texture::Texture: Texture type is not 2D or cube map");
    }
}

Texture::Texture(uint32_t width, uint32_t height, uint32_t depth, GLenum type, GLenum internalFormat, GLsizei mipLevels) : m_type(type), m_width(width), m_height(height), m_depth(depth), m_mipLevels(mipLevels), m_internalFormat(internalFormat) {
    if (width == 0 || height == 0 || depth == 0 || mipLevels > 10 || width < (1 << mipLevels) || height < (1 << mipLevels)) {
        throw std::runtime_error(std::format("Texture::Texture: Invalid Texture size {}x{}x{} for mip level {}", width, height, depth, mipLevels));
    }

    glCreateTextures(m_type, 1, &m_id);

    if (m_type == GL_TEXTURE_3D) {
        glTextureStorage3D(m_id, m_mipLevels, m_internalFormat, m_width, m_height, m_depth);
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
    if (level < 0 || level >= m_mipLevels) {
        throw std::runtime_error(std::format("Texture::upload: mip level {} out of range [0 - {}]", level, m_mipLevels));
    }
    uint32_t width     = img->getWidth();
    uint32_t height    = img->getHeight();
    uint32_t texWidth  = getWidth(level);
    uint32_t texHeight = getHeight(level);
    if (width > texWidth || height > texHeight) {
        throw std::runtime_error(std::format("Texture::upload: image size {}x{} does not match texture size {}x{} at level {}", width, height, texWidth, texHeight, level));
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
}

void Texture::upload(const std::shared_ptr<Image>& img, GLint pos, GLint level) {
    // std::cout << "Uploading texture data from image [" << img->getWidth() << ' ' << img->getHeight() << ' ' << img->getChannels() << ' ' << std::showbase << std::hex << img->getFormat() << ' ' << img->getDataType() << std::dec << "] to level" << level << " GPU memory\n";

    // Check if image data is valid
    if (img == nullptr || img->getData() == nullptr) {
        throw std::runtime_error("Texture::upload: image data is null");
    }
    if (level < 0 || level >= m_mipLevels) {
        throw std::runtime_error(std::format("Texture::upload: mip level {} out of range [0 - {}]", level, m_mipLevels));
    }
    uint32_t width     = img->getWidth();
    uint32_t height    = img->getHeight();
    uint32_t texWidth  = getWidth(level);
    uint32_t texHeight = getHeight(level);
    if (width > texWidth || height > texHeight) {
        throw std::runtime_error(std::format("Texture::upload: image size {}x{} does not match texture size {}x{} at level {}", width, height, texWidth, texHeight, level));
    }

    // Upload texture data to GPU memory
    glTextureSubImage3D(m_id, level, 0, 0, pos, width, height, 1, img->getFormat(), img->getDataType(), img->getData());
}

void Texture::copy(const Texture& other, GLint level) {
    if (m_id == 0 || other.m_id == 0) {
        throw std::runtime_error("Texture::copy: texture ID is null");
    }
    if (level < 0 || level >= m_mipLevels || level >= other.m_mipLevels) {
        throw std::runtime_error(std::format("Texture::copy: mip level {} out of range [0 - {}] or [0 - {}]", level, m_mipLevels - 1, other.m_mipLevels - 1));
    }
    auto srcW = other.getWidth(level);
    auto srcH = other.getHeight(level);
    auto dstW = getWidth(level);
    auto dstH = getHeight(level);
    if (srcW > dstW || srcH > dstH) {
        throw std::runtime_error(std::format("Texture::copy: source texture size {}x{} does not match destination texture size {}x{} at level {}", srcW, srcH, dstW, dstH, level));
    }

    // TODO: gl texture copy api
    // glCopyTextureSubImage2D(m_id, level, 0, 0, srcW, srcH, 0, other.m_id);
}

void Texture::clamp(GLint level) {
    if (m_id == 0) {
        return;
    }
    glTextureParameteri(m_id, GL_TEXTURE_BASE_LEVEL, level);
    glTextureParameteri(m_id, GL_TEXTURE_MAX_LEVEL, level);
}

void Texture::unclamp() {
    if (m_id == 0) {
        return;
    }
    glTextureParameteri(m_id, GL_TEXTURE_BASE_LEVEL, 0);
    glTextureParameteri(m_id, GL_TEXTURE_MAX_LEVEL, m_mipLevels - 1);
}

void Texture::generate() {
    if (m_mipLevels > 1) {
        glGenerateMipmap(m_id);
    }
}

}  // namespace tinyrenderer
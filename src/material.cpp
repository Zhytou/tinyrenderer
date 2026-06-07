#include "material.hpp"

namespace tinyrenderer {

Material::Material(const std::filesystem::path& baseDir, const tinyobj::material_t& material) {
    std::cout << "Loading material [" << material.name << "]\n";

    m_name    = material.name;
    m_opacity = material.dissolve;
    m_albedo  = Texture::create(baseDir / material.diffuse_texname, GL_RGBA8);
    m_normal  = Texture::create(baseDir / material.normal_texname, GL_RGBA8);
    m_mrao    = Texture::create(
        {
            baseDir / material.metallic_texname,
            baseDir / material.roughness_texname,
            baseDir / material.ambient_texname,
        },
        GL_TEXTURE_2D,
        GL_RGBA8,
        1
    );
}

}  // namespace tinyrenderer
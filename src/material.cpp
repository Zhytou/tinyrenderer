#include "material.hpp"

#include "utils.hpp"

namespace tinyrenderer {

Material::Material(const std::filesystem::path& baseDir, const tinyobj::material_t& material) {
    std::cout << "Loading material [" << material.name << "]\n";

    m_name    = material.name;
    m_opacity = material.dissolve;

    if (!material.diffuse_texname.empty()) {
        m_albedo = Texture::create(baseDir / material.diffuse_texname, GL_RGBA8);
    } else {
        m_albedo = Texture::create({material.diffuse[0], material.diffuse[1], material.diffuse[2], 1.0f}, GL_RGBA8);
    }

    if (!material.normal_texname.empty()) {
        m_normal = Texture::create(baseDir / material.normal_texname, GL_RGBA8);
    } else {
        m_normal = Texture::create({0.f, 0.f, 0.f, 0.f}, GL_RGBA8);
    }

    if (!material.metallic_texname.empty() && !material.roughness_texname.empty() && !material.ambient_texname.empty()) {
        m_mrao = Texture::create(
            {
                baseDir / material.metallic_texname,
                baseDir / material.roughness_texname,
                baseDir / material.ambient_texname,
            },
            GL_TEXTURE_2D,
            GL_RGBA8,
            1
        );
    } else {
        // TODO: .mtl file default ambient value support
        m_mrao = Texture::create({material.metallic, material.roughness, 0.f, 0.f}, GL_RGBA8);
    }
}

}  // namespace tinyrenderer
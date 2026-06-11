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
        //! WARNING: Default normal map (0.5, 0.5, 1.0) maps directly to tangent-space vector (0, 0, 1) after
        //! being decoded via [sampled * 2.0 - 1.0] in shader. Since the third column of the TBN matrix is the
        //! vertex normal N, TBN * vec3(0, 0, 1) resolves identically to N. This unifies the rendering pipeline
        //! for meshes without normal maps, eliminating the need for expensive conditional branches (if-else) on GPU.
        m_normal = Texture::create({0.5f, 0.5f, 1.0f, 0.f}, GL_RGBA8);
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
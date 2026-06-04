#pragma once

#include <obj_loader/tiny_obj_loader.h>

#include <filesystem>
#include <iostream>
#include <memory>
#include <stdexcept>

#include "texture.hpp"

namespace tinyrenderer {
class Material {
   public:
    Material() = default;
    Material(const std::filesystem::path& baseDir, const tinyobj::material_t& material);
    ~Material() = default;

    bool isOpaque() const { return m_opacity == 1.0f; }
    float getOpacity() const { return m_opacity; }
    const std::shared_ptr<Texture>& getAlbedoMap() const { return m_albedo; }
    const std::shared_ptr<Texture>& getNormalMap() const { return m_normal; }
    const std::shared_ptr<Texture>& getMRAOMap() const { return m_mrao; }

   private:
    std::string m_name;
    float m_opacity = 1.0f;

    std::shared_ptr<Texture> m_albedo = nullptr;
    std::shared_ptr<Texture> m_normal = nullptr;
    std::shared_ptr<Texture> m_mrao   = nullptr;
};

inline Material::Material(const std::filesystem::path& baseDir, const tinyobj::material_t& material) {
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
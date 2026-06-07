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
    Material(const Material&)            = default;
    Material& operator=(const Material&) = default;
    Material(Material&&)                 = default;
    Material& operator=(Material&&)      = default;
    ~Material()                          = default;

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

}  // namespace tinyrenderer
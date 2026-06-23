#pragma once

#include <obj_loader/tiny_obj_loader.h>

#include <filesystem>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include "texture.hpp"

namespace tinyglrenderer {
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
    std::shared_ptr<Texture> getTexture(const std::string& name) const {
        if (m_textures.count(name)) {
            return m_textures.at(name);
        }
        return nullptr;
    }

   private:
    std::string m_name;
    float m_opacity = 1.0f;
    std::unordered_map<std::string, std::shared_ptr<Texture>> m_textures;
};

}  // namespace tinyglrenderer
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

namespace fs = std::filesystem;

class Material {
   public:
    Material() = default;
    Material(const fs::path& baseDir, const tinyobj::material_t& material);
    Material(const std::unordered_map<std::string, glm::vec4>& values, const std::unordered_map<std::string, fs::path>& textures);
    Material(const Material&)            = default;
    Material& operator=(const Material&) = default;
    Material(Material&&)                 = default;
    Material& operator=(Material&&)      = default;
    ~Material()                          = default;

    bool isOpaque() const { return m_opacity == 1.0f; }
    float getOpacity() const { return m_opacity; }
    std::shared_ptr<Texture> getTexture(const std::string& name) const;
    void setTexture(const std::string& name, const fs::path& path, GLsizei mipLevels = 1);
    void setTexture(const std::string& name, const std::vector<fs::path>& paths, GLsizei mipLevels = 1);
    void setTexture(const std::string& name, const glm::vec4& value, GLsizei mipLevels = 1);
    void setTexture(const std::string& name, const std::shared_ptr<Texture>& texture);

   private:
    std::string m_name;
    float m_opacity = 1.0f;
    std::unordered_map<std::string, std::shared_ptr<Texture>> m_textures;
};

} // namespace tinyglrenderer
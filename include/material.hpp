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
    Material()  = default;
    ~Material() = default;
    
    Material(const std::string& name, float opacity, const std::unordered_map<std::string, std::shared_ptr<Texture>>& textures);

    const std::string& getName() const { return m_name; }
    bool isOpaque() const { return m_opacity > 0.90f; }
    float getOpacity() const { return m_opacity; }
    std::shared_ptr<Texture> getTexture(const std::string& name) const;
    void setOpacity(float opacity) { m_opacity = opacity; }
    void setTexture(const std::string& name, const std::shared_ptr<Texture>& texture);

   private:
    std::string m_name;
    float m_opacity = 1.0f;
    std::unordered_map<std::string, std::shared_ptr<Texture>> m_textures;
};

} // namespace tinyglrenderer
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
    
    Material(const std::string& name, const std::unordered_map<std::string, std::shared_ptr<Texture>>& textures);
    Material(const std::unordered_map<std::string, glm::vec4>& values, const std::unordered_map<std::string, fs::path>& textures);

    std::shared_ptr<Texture> getTexture(const std::string& name) const;
    void setTexture(const std::string& name, const std::shared_ptr<Texture>& texture);

   private:
    std::string m_name;
    std::unordered_map<std::string, std::shared_ptr<Texture>> m_textures;
};

} // namespace tinyglrenderer
#include "material.hpp"

#include <glm/gtc/type_ptr.hpp>

#include "utils.hpp"

namespace tinyglrenderer {

namespace fs = std::filesystem;

Material::Material(const std::string& name, const std::unordered_map<std::string, std::shared_ptr<Texture>>& textures) {
    m_name = name;
    m_textures = textures;
}

std::shared_ptr<Texture> Material::getTexture(const std::string& name) const {
    if (m_textures.count(name)) {
        return m_textures.at(name);
    }
    return nullptr;
}

void Material::setTexture(const std::string& name, const std::shared_ptr<Texture>& texture) {
    m_textures[name] = texture;
}

} // namespace tinyglrenderer
#include "material.hpp"

#include <glm/gtc/type_ptr.hpp>

#include "utils.hpp"

namespace tinyglrenderer {

namespace fs = std::filesystem;

Material::Material(const fs::path& baseDir, const tinyobj::material_t& material) {
    std::cout << "Loading material [" << material.name << "]\n";

    m_name    = material.name;
    m_opacity = material.dissolve;

    // TODO: fix mip level(when miplevel is more than 1, the render result is wrong, blocking artifacts appear)
    const GLsizei mipLevels = 1;
    if (!material.diffuse_texname.empty()) {
        std::cout << "Loading diffuse texture from file [" << material.diffuse_texname << "]\n";
        setTexture("albedo", baseDir / material.diffuse_texname, mipLevels);
    } else {
        std::cout << "Loading diffuse texture from color [" << material.diffuse[0] << ", " << material.diffuse[1] << ", " << material.diffuse[2] << "]\n";
        glm::vec4 color = glm::vec4(material.diffuse[0], material.diffuse[1], material.diffuse[2], 0.0f);
        setTexture("albedo", color, mipLevels);
    }

    if (!material.normal_texname.empty()) {
        std::cout << "Loading normal texture from file [" << material.normal_texname << "]\n";
        setTexture("normal", baseDir / material.normal_texname, mipLevels);
    } else {
        //! WARNING: Default normal map (0.5, 0.5, 1.0) maps directly to tangent-space vector (0, 0, 1) after
        //! being decoded via [sampled * 2.0 - 1.0] in shader. Since the third column of the TBN matrix is the
        //! vertex normal N, TBN * vec3(0, 0, 1) resolves identically to N. This unifies the rendering pipeline
        //! for meshes without normal maps, eliminating the need for expensive conditional branches (if-else) on GPU.
        std::cout << "Loading normal texture from color [0.5, 0.5, 1.0]\n";
        setTexture("normal", glm::vec4(0.5f, 0.5f, 1.0f, 0.f), mipLevels);
    }

    if (!material.metallic_texname.empty() && !material.roughness_texname.empty() && !material.ambient_texname.empty()) {
        std::vector<fs::path> paths = {baseDir / material.metallic_texname, baseDir / material.roughness_texname, baseDir / material.ambient_texname};
        std::cout << "Loading mrao texture from files [";
        for (auto path : paths) {
            std::cout << path << ' ';
        }
        std::cout << "]\n";
        setTexture("mrao", paths, mipLevels);
    } else {
        std::cout << "Loading mrao texture from color [" << material.metallic << ", " << material.roughness << ", 0.0, 0.0]\n";
        glm::vec4 mrao = glm::vec4(material.metallic, material.roughness, 0.f, 0.f);
        setTexture("mrao", mrao, mipLevels);
    }
}

Material::Material(const std::unordered_map<std::string, glm::vec4>& values, const std::unordered_map<std::string, std::filesystem::path>& textures) {
    m_name    = "default";
    m_opacity = 1.0f;

    const GLsizei mipLevels = 1;
    for (auto& [name, value] : values) {
        setTexture(name, value, mipLevels);
    }
    for (auto& [name, path] : textures) {
        setTexture(name, path, mipLevels);
    }
}

std::shared_ptr<Texture> Material::getTexture(const std::string& name) const {
    if (m_textures.count(name)) {
        return m_textures.at(name);
    }
    return nullptr;
}

void Material::setTexture(const std::string& name, const fs::path& path, GLsizei mipLevels) {
    auto image       = Image::create(path, 0, true);
    m_textures[name] = std::make_shared<Texture>(image->getWidth(), image->getHeight(), GL_TEXTURE_2D, GL_RGBA8, mipLevels);
    m_textures[name]->upload(image);
}

void Material::setTexture(const std::string& name, const std::vector<fs::path>& paths, GLsizei mipLevels) {
    std::vector<std::shared_ptr<Image>> images;
    for (auto path : paths) {
        images.push_back(Image::create(path, 1, true));
    }
    auto image       = Image::merge(images, 3);
    m_textures[name] = std::make_shared<Texture>(image->getWidth(), image->getHeight(), GL_TEXTURE_2D, GL_RGBA8, mipLevels);
    m_textures[name]->upload(image);
}

void Material::setTexture(const std::string& name, const glm::vec4& value, GLsizei mipLevels) {
    m_textures[name] = std::make_shared<Texture>(1, 1, GL_TEXTURE_2D, GL_RGBA8, mipLevels);
    m_textures[name]->clear(glm::value_ptr(value), GL_RGBA, GL_FLOAT);
}

void Material::setTexture(const std::string& name, const std::shared_ptr<Texture>& texture) {
    m_textures[name] = texture;
}

} // namespace tinyglrenderer
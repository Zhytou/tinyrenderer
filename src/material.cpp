#include "material.hpp"

#include <glm/gtc/type_ptr.hpp>

#include "utils.hpp"

namespace tinyrenderer {

namespace fs = std::filesystem;

Material::Material(const fs::path& baseDir, const tinyobj::material_t& material) {
    std::cout << "Loading material [" << material.name << "]\n";

    const GLsizei mipLevel = 3;
    m_name                 = material.name;
    m_opacity              = material.dissolve;

    if (!material.diffuse_texname.empty()) {
        std::cout << "Loading diffuse texture from file [" << material.diffuse_texname << "]\n";
        auto image           = Image::create(baseDir / material.diffuse_texname);
        m_textures["albedo"] = std::make_shared<Texture>(image->getWidth(), image->getHeight(), GL_TEXTURE_2D, GL_RGBA8, mipLevel);
        m_textures["albedo"]->upload(image);
    } else {
        std::cout << "Loading diffuse texture from color [" << material.diffuse[0] << ", " << material.diffuse[1] << ", " << material.diffuse[2] << "]\n";
        glm::vec4 color      = glm::vec4(material.diffuse[0], material.diffuse[1], material.diffuse[2], 1.0f);
        m_textures["albedo"] = std::make_shared<Texture>(1, 1, GL_TEXTURE_2D, GL_RGBA8, 1);
        m_textures["albedo"]->clear(glm::value_ptr(color), GL_RGBA, GL_FLOAT);
    }

    if (!material.normal_texname.empty()) {
        std::cout << "Loading normal texture from file [" << material.normal_texname << "]\n";
        auto image           = Image::create(baseDir / material.normal_texname);
        m_textures["normal"] = std::make_shared<Texture>(image->getWidth(), image->getHeight(), GL_TEXTURE_2D, GL_RGBA8, mipLevel);
        m_textures["normal"]->upload(image);
    } else {
        //! WARNING: Default normal map (0.5, 0.5, 1.0) maps directly to tangent-space vector (0, 0, 1) after
        //! being decoded via [sampled * 2.0 - 1.0] in shader. Since the third column of the TBN matrix is the
        //! vertex normal N, TBN * vec3(0, 0, 1) resolves identically to N. This unifies the rendering pipeline
        //! for meshes without normal maps, eliminating the need for expensive conditional branches (if-else) on GPU.
        std::cout << "Loading normal texture from color [0.5, 0.5, 1.0]\n";
        glm::vec4 normal     = glm::vec4(0.5f, 0.5f, 1.0f, 0.f);
        m_textures["normal"] = std::make_shared<Texture>(1, 1, GL_TEXTURE_2D, GL_RGBA8, 1);
        m_textures["normal"]->clear(glm::value_ptr(normal), GL_RGBA, GL_FLOAT);
    }

    if (!material.metallic_texname.empty() && !material.roughness_texname.empty() && !material.ambient_texname.empty()) {
        std::vector<fs::path> paths = {baseDir / material.metallic_texname, baseDir / material.roughness_texname, baseDir / material.ambient_texname};
        std::vector<std::shared_ptr<Image>> images;

        std::cout << "Loading mrao texture from files [";
        for (auto path : paths) {
            images.push_back(Image::create(path));
            std::cout << path << ' ';
        }
        std::cout << "]\n";

        auto image         = Image::merge(images, 3);
        m_textures["mrao"] = std::make_shared<Texture>(image->getWidth(), image->getHeight(), GL_TEXTURE_2D, GL_RGBA8, mipLevel);
        m_textures["mrao"]->upload(image);
    } else {
        std::cout << "Loading mrao texture from color [" << material.metallic << ", " << material.roughness << ", 0.0, 0.0]\n";
        glm::vec4 mrao     = glm::vec4(material.metallic, material.roughness, 0.f, 0.f);
        m_textures["mrao"] = std::make_shared<Texture>(1, 1, GL_TEXTURE_2D, GL_RGBA8, 1);
        m_textures["mrao"]->clear(glm::value_ptr(mrao), GL_RGBA, GL_FLOAT);
    }
}

}  // namespace tinyrenderer
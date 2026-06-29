#pragma once

#include <array>
#include <format>
#include <glm/glm.hpp>
#include <obj_loader/tiny_obj_loader.h>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <filesystem>

#include "image.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "vertexbuffer.hpp"
#include "vertexlayout.hpp"
#include "model.hpp"

namespace tinyglrenderer {

namespace fs = std::filesystem;

class ResourceManager {
   public:
    ResourceManager()  = default;
    ~ResourceManager() = default;

    ResourceManager(const ResourceManager&)            = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
    ResourceManager(ResourceManager&&)                 = delete;
    ResourceManager& operator=(ResourceManager&&)      = delete;

    static const GLsizei& getCounts(const std::string& name) {
        if (!m_counts.count(name)) { throw std::runtime_error("ResourceManager::getCounts: Vertex count for " + name + " not found in ResourceManager"); }
        return m_counts.at(name);
    }
    static const std::shared_ptr<VertexLayout>& getLayout(const std::string& name) {
        if (!m_layouts.count(name)) { throw std::runtime_error("ResourceManager::getLayout: VertexLayout " + name + " not found in ResourceManager"); }
        return m_layouts.at(name);
    }
    static const std::unique_ptr<VertexBuffer>& getBuffer(const std::string& name) {
        if (!m_buffers.count(name)) { throw std::runtime_error("ResourceManager::getBuffer: VertexBuffer " + name + " not found in ResourceManager"); }
        return m_buffers.at(name);
    }
    static glm::mat4 getCaptureMatrix(GLint index) {
        if (index < 0 || index >= 6) { throw std::runtime_error(std::format("ResourceManager::getCaptureMatrix: CaptureMatrix {} not found in ResourceManager", index)); }
        return m_matrixs[index];
    }

    void initialize();
    void destroy();

    std::shared_ptr<Model> loadModel(const fs::path& baseDir, const fs::path& modelName);
    std::shared_ptr<Material> loadMaterial(const fs::path& baseDir, const tinyobj::material_t& material);
    std::shared_ptr<Texture> loadTexture(const fs::path& texPath, const glm::vec4& defaultValue, GLsizei mipLevels = 1);
    std::shared_ptr<Texture> loadTexture(const std::vector<fs::path>& texPaths, const glm::vec4& defaultValue, GLsizei mipLevels = 1);
    std::shared_ptr<Image> loadImage(const fs::path& imagePath, int desiredChannels, bool verticalFlip);

   private:
    static std::unordered_map<std::string, GLsizei> m_counts;
    static std::unordered_map<std::string, std::shared_ptr<VertexLayout>> m_layouts;
    static std::unordered_map<std::string, std::unique_ptr<VertexBuffer>> m_buffers;
    static std::array<glm::mat4, 6> m_matrixs;

    std::unordered_map<std::string, std::weak_ptr<Mesh>> m_meshes;
    std::unordered_map<std::string, std::weak_ptr<Material>> m_materials;
    std::unordered_map<std::string, std::weak_ptr<Texture>> m_textures;
    std::unordered_map<std::string, std::weak_ptr<Image>> m_images;
    std::unordered_map<std::string, std::weak_ptr<Shader>> m_shaders;
};

}; // namespace tinyglrenderer

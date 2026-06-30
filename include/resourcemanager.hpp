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

    static const GLsizei& getCount(const std::string& name);
    static const std::shared_ptr<VertexLayout>& getLayout(const std::string& name);
    static const std::unique_ptr<VertexBuffer>& getBuffer(const std::string& name);
    static const glm::mat4& getCaptureMatrix(GLint index);
    std::shared_ptr<Mesh> getMesh(const std::string& name) const;
    std::shared_ptr<Texture> getTexture(const std::string& name) const;
    std::shared_ptr<Shader> getShader(const std::string& name) const;
    void getAllMeshNames(std::vector<std::string>& names) const;
    void getAllTextureNames(std::vector<std::string>& names) const;
    void getAllShaderNames(std::vector<std::string>& names) const;

    void initialize();
    void destroy();

    std::shared_ptr<Model> loadModel(const std::string& modelName, const fs::path& objPath, const fs::path& mtlDir);
    std::shared_ptr<Mesh> loadMesh(const std::string& meshName, const fs::path& meshPath, const tinyobj::attrib_t& attributes, const std::vector<tinyobj::shape_t>& shapes, size_t num);
    std::shared_ptr<Material> loadMaterial(const std::string& matName, const fs::path& matDir, const tinyobj::material_t& material);
    std::shared_ptr<Texture> load2DTexture(const std::string& texName, const fs::path& texPath, const glm::vec4& defaultValue, GLenum internalFormat = GL_RGBA8, GLsizei mipLevels = 1, int desiredChannels = 0, bool verticalFlip = true);
    std::shared_ptr<Texture> load2DTexture(const std::string& texName, const std::vector<fs::path>& texPaths, const glm::vec4& defaultValue, GLenum internalFormat = GL_RGBA8, GLsizei mipLevels = 1, int desiredChannels = 0, bool verticalFlip = true);
    std::shared_ptr<Texture> loadCubeTexture(const std::string& texName, const std::vector<fs::path>& texPaths, const glm::vec4& defaultValue, GLenum internalFormat = GL_RGBA8, GLsizei mipLevels = 1, int desiredChannels = 0, bool verticalFlip = true);
    std::shared_ptr<Image> loadImage(const std::string& imageName, const fs::path& imagePath, int desiredChannels = 0, bool verticalFlip = true);
    std::shared_ptr<Shader> loadShader(const std::string& shaderName, const fs::path& vertexShaderPath, const fs::path& fragmentShaderPath);

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

    const GLsizei m_textureDefaultWidth = 256;
    const GLsizei m_textureDefaultHeight = 256;
};

}; // namespace tinyglrenderer

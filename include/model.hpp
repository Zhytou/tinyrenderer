#pragma once

#include <filesystem>
#include <glm/glm.hpp>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "material.hpp"
#include "mesh.hpp"
#include "renderitem.hpp"

namespace tinyrenderer {

struct alignas(16) ModelBlock {
    glm::mat4 transformMatrix;  // translate, rotate, scale
    glm::mat4 normalMatrix;     // transpose of inverse (roate * scale) matrix
};

class Model {
   public:
    Model()                        = default;
    Model(const Model&)            = delete;
    Model& operator=(const Model&) = delete;
    Model(Model&& other);
    Model& operator=(Model&& other);
    Model(const std::filesystem::path& baseDir, const std::filesystem::path& modelName, const glm::mat4& transform);
    ~Model();

    const std::pair<glm::vec3, glm::vec3>& getBoundingBox() const { return m_mesh->getBoundingBox(); }
    const ModelBlock& getModelBlock() const { return m_modelBlock; }
    void getRenderQueue(std::vector<RenderItem>& queue, bool opaque) const;
    void setTransformMatrix(const glm::mat4& transform) {
        m_modelBlock.transformMatrix = transform;
        m_modelBlock.normalMatrix    = glm::mat4(glm::transpose(glm::inverse(glm::mat3(transform))));
    }

   private:
    std::vector<std::shared_ptr<Material>> m_materials;
    std::unique_ptr<Mesh> m_mesh;

    ModelBlock m_modelBlock = {glm::mat4(1.f), glm::mat4(1.f)};
};

}  // namespace tinyrenderer

#pragma once

#include <filesystem>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "material.hpp"
#include "mesh.hpp"
#include "renderitem.hpp"

namespace tinyglrenderer {

struct alignas(16) ModelBlock {
    glm::mat4 transformMatrix; // translate, rotate, scale
    glm::mat4 normalMatrix;    // transpose of inverse (roate * scale) matrix
    // glm::mat4 padding1;
    // glm::mat4 padding2;
};

class Model {
   public:
    Model();
    Model(const std::string& name, const std::shared_ptr<Mesh>& mesh, const std::vector<std::shared_ptr<Material>>& materials, const std::shared_ptr<Material>& defaultMaterial = nullptr);
    ~Model();

    Model(const Model&)            = delete;
    Model& operator=(const Model&) = delete;
    Model(Model&& other);
    Model& operator=(Model&& other);

    const std::string& getName() const { return m_name; }
    const std::pair<glm::vec3, glm::vec3>& getBoundingBox() const { return m_bounds; }
    const ModelBlock& getModelBlock() const { return m_modelBlock; }
    void getRenderQueue(std::vector<RenderItem>& queue, bool opaque) const;
    const glm::vec3& getTranslate() const { return m_transforms.at("translate"); }
    const glm::vec3& getRotate() const { return m_transforms.at("rotate"); }
    const glm::vec3& getScale() const { return m_transforms.at("scale"); }
    void setDefaultMaterial(const std::shared_ptr<Material>& material) { m_material = material; }
    void setTransform(const glm::vec3& translate, const glm::vec3& rotate, const glm::vec3& scale);

   private:
    std::string m_name;
    std::vector<std::shared_ptr<Material>> m_materials;
    std::shared_ptr<Material> m_material; // default material
    std::shared_ptr<Mesh> m_mesh;

    std::pair<glm::vec3, glm::vec3> m_bounds = {
        glm::vec3(FLT_MAX),
        glm::vec3(-FLT_MAX),
    };
    std::unordered_map<std::string, glm::vec3> m_transforms = {
        {"translate", glm::vec3(0.0f)},
        {"rotate", glm::vec3(0.0f)},
        {"scale", glm::vec3(1.0f)},
    };
    ModelBlock m_modelBlock = {
        .transformMatrix = glm::mat4(1.f),
        .normalMatrix    = glm::mat4(1.f),
    };
};

} // namespace tinyglrenderer

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
    Model()                        = default;
    Model(const Model&)            = delete;
    Model& operator=(const Model&) = delete;
    Model(Model&& other);
    Model& operator=(Model&& other);
    Model(const std::filesystem::path& baseDir, const std::filesystem::path& modelName, const std::shared_ptr<Material>& defaultMat);
    ~Model();

    const std::string& getName() const { return m_name; }
    const std::pair<glm::vec3, glm::vec3>& getBoundingBox() const { return m_bounds; }
    const ModelBlock& getModelBlock() const { return m_modelBlock; }
    void getRenderQueue(std::vector<RenderItem>& queue, bool opaque) const;
    void setTransform(const glm::vec3& translate, const glm::vec3& rotate, const glm::vec3& scale) {
        glm::mat4 transform = glm::mat4(1.0f);

        transform = glm::translate(transform, translate);
        transform = glm::rotate(transform, glm::radians(rotate.x), glm::vec3(1.0f, 0.0f, 0.0f));
        transform = glm::rotate(transform, glm::radians(rotate.y), glm::vec3(0.0f, 1.0f, 0.0f));
        transform = glm::rotate(transform, glm::radians(rotate.z), glm::vec3(0.0f, 0.0f, 1.0f));
        transform = glm::scale(transform, scale);

        // update transforms
        m_transforms["translate"] = translate;
        m_transforms["rotate"]    = rotate;
        m_transforms["scale"]     = scale;

        // update transform matrix
        m_modelBlock.transformMatrix = transform;
        m_modelBlock.normalMatrix    = glm::mat4(glm::transpose(glm::inverse(glm::mat3(transform))));

        // update bounding box
        auto [xyz1, xyz2] = m_mesh->getBoundingBox();
        xyz1              = glm::vec3(transform * glm::vec4(xyz1, 1.f));
        xyz2              = glm::vec3(transform * glm::vec4(xyz2, 1.f));
        m_bounds          = {xyz1, xyz2};
    }

   private:
    std::string m_name;
    std::vector<std::shared_ptr<Material>> m_materials;
    std::unique_ptr<Mesh> m_mesh;

    std::pair<glm::vec3, glm::vec3> m_bounds = {
        glm::vec3(FLT_MAX),
        glm::vec3(-FLT_MAX),
    };
    std::unordered_map<std::string, glm::vec3> m_transforms = {
        {"translate", glm::vec3(0.0f)},
        {"rotate",    glm::vec3(0.0f)},
        {"scale",     glm::vec3(1.0f)},
    };
    ModelBlock m_modelBlock = {
        .transformMatrix = glm::mat4(1.f),
        .normalMatrix    = glm::mat4(1.f),
    };
};

} // namespace tinyglrenderer

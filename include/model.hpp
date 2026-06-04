#pragma once

#include <filesystem>
#include <glm/glm.hpp>
#include <map>
#include <string>

#include "material.hpp"
#include "renderitem.hpp"

namespace tinyrenderer {

struct Vertex {
    glm::vec3 position;  // position
    glm::vec3 normal;    // normal
    glm::vec3 tangent;   // tangent
    glm::vec2 texcoord;  // texture coordinate
};

struct SubMesh {
    std::string mname;    // material name
    uint32_t offset = 0;  // start index in indices vector
    uint32_t length = 0;  // length of indices in submesh
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

    const std::pair<glm::vec3, glm::vec3>& getBoundingBox() const { return m_xyz; }
    const glm::mat4& getTransformMatrix() const { return m_matrix; }
    void genRenderQueue(std::vector<RenderItem>& queue, bool opaque) const {
        for (auto& sm : m_submeshes) {
            if (m_materials.at(sm.mname).isOpaque() != opaque) {
                continue;
            }
            queue.emplace_back(m_vao, sm.length, sm.offset, 0.f, m_matrix, m_materials.at(sm.mname));
        }
    }
    void setTransformMatrix(const glm::mat4& transform) { m_matrix = transform; }

   private:
    std::vector<Vertex> m_vertices;
    std::vector<uint32_t> m_indices;
    std::vector<SubMesh> m_submeshes;
    std::map<std::string, Material> m_materials;

    glm::mat4 m_matrix                    = glm::mat4(1.f);
    std::pair<glm::vec3, glm::vec3> m_xyz = {glm::vec3(0.f), glm::vec3(0.f)};

    GLuint m_vao = 0;
    GLuint m_vbo = 0;
    GLuint m_ebo = 0;
};

}  // namespace tinyrenderer

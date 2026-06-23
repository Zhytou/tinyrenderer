#pragma once

#include <obj_loader/tiny_obj_loader.h>

#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

#include "indexbuffer.hpp"
#include "vertexbuffer.hpp"
#include "vertexlayout.hpp"

namespace tinyglrenderer {

struct Vertex {
    glm::vec3 position;  // position
    glm::vec3 normal;    // normal
    glm::vec3 tangent;   // tangent
    glm::vec2 texcoord;  // texture coordinate
};

struct SubMesh {
    uint32_t matid;       // material id(index in m_materials vector of model)
    uint32_t offset = 0;  // start index in indices vector
    uint32_t length = 0;  // length of indices in submesh
};

class Mesh {
   public:
    Mesh(const tinyobj::attrib_t& attributes, const std::vector<tinyobj::shape_t>& shapes, size_t numMaterials);
    Mesh(const Mesh&)            = delete;
    Mesh& operator=(const Mesh&) = delete;
    ~Mesh();

    const std::pair<glm::vec3, glm::vec3>& getBoundingBox() const { return m_bounds; }
    const std::vector<SubMesh>& getSubMeshes() const { return m_submeshes; }
    const std::shared_ptr<VertexLayout>& getVertexLayout() const { return m_layout; }
    const std::unique_ptr<VertexBuffer>& getVertexBuffer() const { return m_bufferv; }
    const std::unique_ptr<IndexBuffer>& getIndexBuffer() const { return m_bufferi; }

   private:
    std::shared_ptr<VertexLayout> m_layout;             // vertex array object
    std::unique_ptr<VertexBuffer> m_bufferv = nullptr;  // vertex buffer object
    std::unique_ptr<IndexBuffer> m_bufferi  = nullptr;  // index buffer object
    std::vector<SubMesh> m_submeshes;

    std::pair<glm::vec3, glm::vec3> m_bounds = {glm::vec3(FLT_MAX), glm::vec3(-FLT_MAX)};
};

}  // namespace tinyglrenderer
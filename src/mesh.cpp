#include "mesh.hpp"

#include <glm/glm.hpp>

#include "staticresource.hpp"

namespace tinyrenderer {

Mesh::Mesh(const tinyobj::attrib_t& attributes, const std::vector<tinyobj::shape_t>& shapes, size_t numMaterials) {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    // 1. Traverse tinyobj loading data and initialize vertices
    std::vector<std::vector<uint32_t>> submeshes(numMaterials);
    for (auto& shape : shapes) {
        for (int i = 0; i < shape.mesh.material_ids.size(); i++) {
            glm::vec3 vertex[3], normal[3], tangent;
            glm::vec2 uv[3];

            for (int j = 0; j < 3; ++j) {
                tinyobj::index_t index = shape.mesh.indices[3 * i + j];
                int v = index.vertex_index, n = index.normal_index, t = index.texcoord_index;

                vertex[j] = glm::vec3(attributes.vertices[3 * v], attributes.vertices[3 * v + 1], attributes.vertices[3 * v + 2]);
                if (n >= 0) {
                    normal[j] = glm::vec3(attributes.normals[3 * n], attributes.normals[3 * n + 1], attributes.normals[3 * n + 2]);
                }
                if (t >= 0) {
                    uv[j] = glm::vec2(attributes.texcoords[2 * t], attributes.texcoords[2 * t + 1]);
                }
                m_bounds.first  = glm::min(m_bounds.first, vertex[j]);
                m_bounds.second = glm::max(m_bounds.second, vertex[j]);
            }

            {
                glm::vec3 edge1 = vertex[1] - vertex[0], edge2 = vertex[2] - vertex[1];
                glm::vec2 deltaUV1 = uv[1] - uv[0], deltaUV2 = uv[2] - uv[1];

                float divide = deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y;
                if (std::abs(divide) < 1e-6f) divide = (divide >= 0 ? 1e-6f : -1e-6f);
                float df  = 1.0f / divide;
                tangent.x = df * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
                tangent.y = df * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
                tangent.z = df * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
                tangent   = glm::normalize(tangent);
            }

            int mat_id = shape.mesh.material_ids[i];
            for (int j = 0; j < 3; ++j) {
                vertices.emplace_back(vertex[j], normal[j], tangent, uv[j]);
                if (mat_id >= 0) {
                    submeshes[mat_id].push_back(static_cast<unsigned int>(vertices.size() - 1));
                }
            }
        }
    }

    // 2. Group geometry into submeshes by material, and concatenate all index data into m_indices
    for (int i = 0; i < submeshes.size(); i++) {
        if (!submeshes[i].empty()) {
            m_submeshes.emplace_back(i, indices.size(), submeshes[i].size());
            indices.insert(indices.end(), submeshes[i].begin(), submeshes[i].end());
        }
    }

    // 3.Create VAO/VBO/IBO and configure vertex attributes
    m_layout  = StaticResource::getInstance().getLayout("mesh");
    m_bufferv = std::make_unique<VertexBuffer>(vertices.size() * sizeof(Vertex), vertices.data());
    m_bufferi = std::make_unique<IndexBuffer>(indices.size() * sizeof(uint32_t), indices.data());
}

Mesh::~Mesh() {
    if (m_layout) m_layout.reset();
    if (m_bufferv) m_bufferv.reset();
    if (m_bufferi) m_bufferi.reset();
}
};  // namespace tinyrenderer
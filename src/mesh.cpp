#include "mesh.hpp"

#include <glm/glm.hpp>

#include "resourcemanager.hpp"

namespace tinyglrenderer {

Mesh::Mesh(const fs::path& path, const tinyobj::attrib_t& attributes, const std::vector<tinyobj::shape_t>& shapes, size_t num) {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    // 1. Traverse tinyobj loading data and initialize vertices
    m_filepath = path;
    std::vector<std::vector<uint32_t>> submeshes(num + 1); // num is material count
    for (auto& shape : shapes) {
        for (int i = 0; i < shape.mesh.material_ids.size(); i++) { // i is triangle index
            bool vn = true, vt = true;
            glm::vec3 vertex[3], normal[3], tangent;
            glm::vec2 uv[3];

            for (int j = 0; j < 3; ++j) { // j is vertex index
                tinyobj::index_t index = shape.mesh.indices[3 * i + j];
                int v = index.vertex_index, n = index.normal_index, t = index.texcoord_index;

                vertex[j] = glm::vec3(attributes.vertices[3 * v], attributes.vertices[3 * v + 1], attributes.vertices[3 * v + 2]);
                if (n >= 0) { normal[j] = glm::vec3(attributes.normals[3 * n], attributes.normals[3 * n + 1], attributes.normals[3 * n + 2]); }
                if (t >= 0) { uv[j] = glm::vec2(attributes.texcoords[2 * t], attributes.texcoords[2 * t + 1]); }
                vn = (n >= 0) && vn;
                vt = (t >= 0) && vt;
                m_bounds.first  = glm::min(m_bounds.first, vertex[j]);
                m_bounds.second = glm::max(m_bounds.second, vertex[j]);
            }

            if (!vn) {
                glm::vec3 edge1 = vertex[1] - vertex[0], edge2 = vertex[2] - vertex[0]; // different from tangent calculation, share the same end
                glm::vec3 fnormal = glm::normalize(glm::cross(edge1, edge2)); // face normal
                for (int j = 0; j < 3; ++j) { normal[j] = fnormal;}
            }

            if (!vt) {
                for (int j = 0; j < 3; ++j) { uv[j] = glm::vec2(0.0f, 0.0f);}
                tangent = glm::vec3(0.0f, 1.0f, 0.0f);
            }

            if (vt) { // only calculate tangent if uv is available, otherwise set tangent to 0, 1, 0
                glm::vec3 edge1 = vertex[1] - vertex[0], edge2 = vertex[2] - vertex[1]; // different from face normal calculation, end to end
                glm::vec2 deltaUV1 = uv[1] - uv[0], deltaUV2 = uv[2] - uv[1];

                float divide = deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y;
                if (std::abs(divide) < 1e-6f) { divide = (divide >= 0 ? 1e-6f : -1e-6f); }
                float df  = 1.0f / divide;
                tangent.x = df * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
                tangent.y = df * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
                tangent.z = df * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
                tangent   = glm::normalize(tangent);
            }

            int id = shape.mesh.material_ids[i]; // id is material index
            for (int j = 0; j < 3; ++j) {
                vertices.emplace_back(vertex[j], normal[j], tangent, uv[j]);
                if (id >= 0 && id < num) { submeshes[id].push_back(static_cast<unsigned int>(vertices.size() - 1)); }
                if (id == -1) {
                    // id == -1 means no material assigned, retain these vertices into the trailing
                    submeshes[num].push_back(static_cast<unsigned int>(vertices.size() - 1));
                }
            }
        }
    }

    // 2. Group geometry into submeshes by material, and concatenate all index data into m_indices
    for (int id = 0; id <= num; id++) { // id is material index
        auto& submesh = submeshes[id];
        if (!submesh.empty()) {
            m_submeshes.emplace_back(id < num ? id : -1, indices.size(), submesh.size());
            indices.insert(indices.end(), submesh.begin(), submesh.end());
        }
    }

    // 3.Create VAO/VBO/IBO and configure vertex attributes
    m_layout  = ResourceManager::getLayout("mesh");
    m_bufferv = std::make_unique<VertexBuffer>(vertices.size() * sizeof(Vertex), vertices.data());
    m_bufferi = std::make_unique<IndexBuffer>(indices.size() * sizeof(uint32_t), indices.data());
}

Mesh::~Mesh() {
    if (m_layout) { m_layout.reset(); }
    if (m_bufferv) { m_bufferv.reset(); }
    if (m_bufferi) { m_bufferi.reset(); }
}

}; // namespace tinyglrenderer
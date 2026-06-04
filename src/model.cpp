#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <obj_loader/tiny_obj_loader.h>

#include <iostream>
#include <stdexcept>
#include <unordered_map>
#include <vector>

#include "model.hpp"

namespace tinyrenderer {

namespace fs = std::filesystem;

Model::Model(Model&& other) : m_vertices(std::move(other.m_vertices)),
                              m_indices(std::move(other.m_indices)),
                              m_submeshes(std::move(other.m_submeshes)),
                              m_materials(std::move(other.m_materials)),
                              m_matrix(other.m_matrix),
                              m_xyz(other.m_xyz),
                              m_vao(other.m_vao),
                              m_vbo(other.m_vbo),
                              m_ebo(other.m_ebo) {
    other.m_vao = other.m_vbo = other.m_ebo = 0;
}
Model& Model::operator=(Model&& other) {
    if (this != &other) {
        if (m_vao) glDeleteVertexArrays(1, &m_vao);
        if (m_vbo) glDeleteBuffers(1, &m_vbo);
        if (m_ebo) glDeleteBuffers(1, &m_ebo);

        m_vertices  = std::move(other.m_vertices);
        m_indices   = std::move(other.m_indices);
        m_submeshes = std::move(other.m_submeshes);
        m_materials = std::move(other.m_materials);
        m_matrix    = other.m_matrix;
        m_xyz       = other.m_xyz;
        m_vao       = other.m_vao;
        m_vbo       = other.m_vbo;
        m_ebo       = other.m_ebo;

        other.m_vao = other.m_vbo = other.m_ebo = 0;
    }
    return *this;
}

Model::Model(const fs::path& baseDir, const fs::path& modelName, const glm::mat4& transform) {
    std::cout << "Loading model [" << baseDir / modelName << "]\n";

    // 1. Load obj model with tinyobj loader
    fs::path modelPath = baseDir / modelName;
    tinyobj::attrib_t attributes;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string err;
    if (!tinyobj::LoadObj(&attributes, &shapes, &materials, &err, modelPath.c_str(), baseDir.c_str(), true)) {
        throw std::runtime_error(err);
    }

    // 2. Traverse tinyobj loading data and initialize m_vertices
    std::vector<std::vector<uint32_t>> submeshes(materials.size());
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
                m_xyz.first  = glm::min(m_xyz.first, vertex[j]);
                m_xyz.second = glm::max(m_xyz.second, vertex[j]);
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
                m_vertices.emplace_back(vertex[j], normal[j], tangent, uv[j]);
                if (mat_id >= 0) {
                    submeshes[mat_id].push_back(static_cast<unsigned int>(m_vertices.size() - 1));
                }
            }
        }
    }

    // 3. Group geometry into submeshes by material, and concatenate all index data into m_indices
    for (int i = 0; i < submeshes.size(); i++) {
        if (!submeshes[i].empty()) {
            m_submeshes.emplace_back(materials[i].name, m_indices.size(), submeshes[i].size());
            m_indices.insert(m_indices.end(), submeshes[i].begin(), submeshes[i].end());
        }
    }

    // 4. Convert tinyobj material into m_materials map
    for (auto& material : materials) {
        m_materials[material.name] = Material(baseDir, material);
    }

    // 5. Initialize model transform m_matrix
    m_matrix = transform;

    // 6. Create vertex array object and vertex buffer object, element buffer object for model data
    {
        // generate and bind vertex array object
        glGenVertexArrays(1, &m_vao);
        glBindVertexArray(m_vao);

        // generate and bind vertex buffer object
        glGenBuffers(1, &m_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        // fill vbo with vertex data
        glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), m_vertices.data(), GL_STATIC_DRAW);

        // generate and bind element buffer object
        glGenBuffers(1, &m_ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
        // fill ebo with index data
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(uint32_t), m_indices.data(), GL_STATIC_DRAW);

        // set vertex attribute pointers which describe how to interpret vertex data(use in vertex shader by location = 0, 1, 2, 3)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texcoord));
        glEnableVertexAttribArray(3);

        // unbind vao
        glBindVertexArray(0);
    }
}

Model::~Model() {
    if (m_vao) {
        glDeleteVertexArrays(1, &m_vao);
        m_vao = 0;
    }
    if (m_vbo) {
        glDeleteBuffers(1, &m_vbo);
        m_vbo = 0;
    }
    if (m_ebo) {
        glDeleteBuffers(1, &m_ebo);
        m_ebo = 0;
    }

    m_materials.clear();
}

}  // namespace tinyrenderer

#pragma once

#include <vector>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <tiny_obj_loader.h>

namespace tinyrenderer {

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texcoord;
};

struct Face {
    Face() = default;
    // vertex index
    int p1, p2, p3;
};

struct Mesh {
    Mesh() = default;
    Mesh(std::vector<Vertex>&& vs, std::vector<Face>&& fs, int mid);

    std::vector<Vertex> vertices;
    std::vector<Face> faces;
    int materialId;

    static GLuint vao;
    GLuint vbo;
    GLuint ebo;
};

using Material = tinyobj::material_t;

struct Model {
    Model() = default;
    Model(const std::string& path, const std::string& name);

    std::vector<Mesh> meshes;
    std::vector<Material> materials;
};

} // namespace tinyrenderer


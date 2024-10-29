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
    Face(int i1, int i2, int i3) : p1(i1), p2(i2), p3(i3) {}
    // vertex index
    int p1, p2, p3;
};

struct Mesh {
    Mesh() = default;
    Mesh(std::vector<Vertex>&& vs, std::vector<Face>&& fs, int mid);

    std::vector<Vertex> vertices;
    std::vector<Face> faces;
    int materialId;

    GLuint vao;
    GLuint vbo;
    GLuint ebo;
};

using Material = tinyobj::material_t;

class Model {
    Model() = default;
    Model(const std::string& path, const std::string& name);

    std::vector<Mesh> meshes;
    std::vector<Material> materials;
    std::vector<GLuint> ubos;
};

} // namespace tinyrenderer


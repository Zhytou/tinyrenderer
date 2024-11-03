#pragma once

#include <vector>
#include <map>

#include <glad/glad.h>
#include <glm/glm.hpp>

namespace tinyrenderer {

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec2 texcoord;
};

struct Mesh {
    Mesh() = default;
    Mesh(std::vector<Vertex>&& vs, std::vector<unsigned int>&& is);

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    GLuint vao;
    GLuint vbo;
    GLuint ebo;
};

struct AABB {
    AABB() : minPos(std::numeric_limits<float>::max()), maxPos(std::numeric_limits<float>::min()) {}
    glm::vec3 minPos;
    glm::vec3 maxPos;
};

struct Texture {
    Texture() = default;
    Texture(const std::string& name);

    GLuint id;
};

struct Model {
    Model() = default;
    Model(const std::map<std::string, std::string>& config);

    AABB aabb;
    Mesh mesh;
    std::map<std::string, Texture> textures; 
};

} // namespace tinyrenderer


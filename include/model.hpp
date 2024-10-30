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
    Mesh(std::vector<Vertex>&& vs, std::vector<int>&& is);

    std::vector<Vertex> vertices;
    std::vector<int> indices;

    GLuint vao;
    GLuint vbo;
    GLuint ebo;
};

struct Texture {
    Texture() = default;
    Texture(const std::string& name);

    GLuint id;
    unsigned char* data;
    int width, height, channels;
};

struct Model {
    Model() = default;
    Model(const std::map<std::string, std::string>& config);

    Mesh mesh;
    Texture albedo;
    Texture normal;
    Texture metallic;
    Texture roughness;
};

} // namespace tinyrenderer


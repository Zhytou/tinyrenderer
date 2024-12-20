#include <vector>
#include <stdexcept>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <glm/gtc/matrix_transform.hpp>

#include "model.hpp"

namespace tinyrenderer
{

Mesh::Mesh(std::vector<Vertex>&& vs, std::vector<unsigned int>&& is) : vertices(vs), indices(is)
{
    // generate and bind vertex array object
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // generate and bind vertex buffer object
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    // fill vbo with vertex data
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    // generate and bind element buffer object
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    // fill ebo with index data
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // set vertex attribute pointers which describe how to interpret vertex data(use in vertex shader by location = 0, 1, 2)
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

Texture::Texture(const std::string& name)
{
    stbi_set_flip_vertically_on_load(true);  

    int width, height, channels;
    unsigned char* data = stbi_load(name.c_str(), &width, &height, &channels, 0);
    if (!data) {
        throw std::runtime_error("Failed to load texture: " + name);
    }

    std::printf("Loading texture: %s %d %d %d\n", name.c_str(), width, height, channels);

    GLenum format;
    if (channels == 1)
        format = GL_RED;
    else if (channels == 3)
        format = GL_RGB;
    else if (channels == 4)
        format = GL_RGBA;
    else
        throw std::runtime_error("Unknown number of channels: " + std::to_string(channels));

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    // set the texture wrapping parameters for both x and y axes
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters when scaling
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // set texture data
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);
}

Model::Model(const std::string baseDir, const std::string modelName, const std::map<std::string, std::string>& texNames, const std::map<std::string, glm::vec3>& transform)
{
    std::string modelPath = baseDir + modelName;

    tinyobj::attrib_t attributes;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string err;
    if (!tinyobj::LoadObj(&attributes, &shapes, &materials, &err, modelPath.c_str(), baseDir.c_str(), true)) {
        throw std::runtime_error(err);
    }

    int totNumFaces = 0;
    std::vector<unsigned int> indices;
    std::vector<Vertex> vertices;
    for (auto& shape : shapes) {        
        // since tinyobj::LoadObj() has already triangulated the mesh, so we can assert that the number of indices is a multiple of 3
        assert(shape.mesh.indices.size() % 3 == 0);
        int numFaces = shape.mesh.indices.size() / 3;
        totNumFaces += numFaces;

        // both of the following sizes represent the number of triangle faces
        assert(numFaces == shape.mesh.num_face_vertices.size() && numFaces == shape.mesh.material_ids.size());

        for (int i = 0; i < numFaces; ++i) {
            // mesh is triangulated
            assert(shape.mesh.num_face_vertices[i] == 3);
            
            bool hasNormal = true;
            bool hasUV = true;

            glm::vec3 vertex[3];
            glm::vec3 normal[3];
            glm::vec3 tangent;
            glm::vec2 uv[3];

            for (int j = 0; j < 3; ++j) {
                tinyobj::index_t index = shape.mesh.indices[3 * i + j];
                int v = index.vertex_index;
                int n = index.normal_index;
                int t = index.texcoord_index;
                
                vertex[j] = glm::vec3(attributes.vertices[3 * v], attributes.vertices[3 * v + 1], attributes.vertices[3 * v + 2]);
                if (n >= 0) {
                    normal[j] = glm::vec3(attributes.normals[3 * n], attributes.normals[3 * n + 1], attributes.normals[3 * n + 2]);
                } else {
                    hasNormal = false;
                }
                if (t >= 0) {
                    uv[j] = glm::vec2(attributes.texcoords[2 * t], attributes.texcoords[2 * t + 1]);
                } else {
                    hasUV = false;
                }

                // update AABB
                {
                    aabb.maxPos.x = std::max(aabb.maxPos.x, vertex[j].x);
                    aabb.maxPos.y = std::max(aabb.maxPos.y, vertex[j].y);
                    aabb.maxPos.z = std::max(aabb.maxPos.z, vertex[j].z);
                    aabb.minPos.x = std::min(aabb.minPos.x, vertex[j].x);
                    aabb.minPos.y = std::min(aabb.minPos.y, vertex[j].y);
                    aabb.minPos.z = std::min(aabb.minPos.z, vertex[j].z);
                }
            }

            if (!hasNormal) {
                glm::vec3 n = glm::normalize(glm::cross(vertex[1] - vertex[0], vertex[2] - vertex[0]));
                for (int j = 0; j < 3; ++j) {
                    normal[j] = n;
                }
            }

            if (!hasUV) {
                for (int j = 0; j < 3; ++j) {
                    uv[j] = glm::vec2(0.5f);
                }
            }

            {
                glm::vec3 edge1    = vertex[1] - vertex[0];
                glm::vec3 edge2    = vertex[2] - vertex[1];
                glm::vec2 deltaUV1 = uv[1] - uv[0];
                glm::vec2 deltaUV2 = uv[2] - uv[1];

                auto divide = deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y;
                if (divide >= 0.0f && divide < 0.000001f)
                    divide = 0.000001f;
                else if (divide < 0.0f && divide > -0.000001f)
                    divide = -0.000001f;

                float df = 1.0f / divide;
                tangent.x = df * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
                tangent.y = df * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
                tangent.z = df * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
                tangent = glm::normalize(tangent);
            }

            for (int j = 0; j < 3; ++j) {
                vertices.push_back({vertex[j], normal[j], tangent, uv[j]});
                indices.push_back(i * 3 + j);
            }
        }
    }

    std::printf("Loading model: %s %d\n", modelName.c_str(), totNumFaces);

    // initialize mesh
    mesh = Mesh(std::move(vertices), std::move(indices));
    // initialize textures
    for (auto [name, path]: texNames) {
        textures[name] = Texture(baseDir + path);
    }
    // initialize model matrix
    modelMatrix = glm::mat4(1.0f);
    for (auto [name, value]: transform) {
        if (name == "translate") {
            modelMatrix = glm::translate(modelMatrix, value);
        } else if (name == "rotate") {
            modelMatrix = glm::rotate(modelMatrix, glm::radians(value.x), glm::vec3(1.0f, 0.0f, 0.0f));
            modelMatrix = glm::rotate(modelMatrix, glm::radians(value.y), glm::vec3(0.0f, 1.0f, 0.0f));
            modelMatrix = glm::rotate(modelMatrix, glm::radians(value.z), glm::vec3(0.0f, 0.0f, 1.0f));
        } else if (name == "scale") {
            modelMatrix = glm::scale(modelMatrix, value);
        } else {
            throw std::runtime_error("Unknown transformation: " + name);
        }
    }
}

} // namespace tinyrenderer


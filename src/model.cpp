#include <vector>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <numeric>
#include <tuple>

#define TINYOBJLOADER_IMPLEMENTATION
#include "model.hpp"

namespace tinyrenderer
{

Mesh::Mesh(std::vector<Vertex>&& vs, std::vector<Face>&& fs, int mid) : vertices(vs), faces(fs), materialId(mid)
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
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * sizeof(Face), faces.data(), GL_STATIC_DRAW);

    // set vertex attribute pointers which describe how to interpret vertex data(use in vertex shader by location = 0, 1, 2)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texcoord));
    glEnableVertexAttribArray(2);
}

Model::Model(const std::string& path, const std::string& name)
{
    tinyobj::attrib_t attributes;
    std::vector<tinyobj::shape_t> shapes;
    std::string err;
    std::string fullName = path + name;
    if (!tinyobj::LoadObj(&attributes, &shapes, &materials, &err, fullName.c_str(), path.c_str(), true)) {
        throw std::runtime_error(err);
    }

    for (const auto& material : materials) {
        GLuint ubo;
        glGenBuffers(1, &ubo);
        glBindBuffer(GL_UNIFORM_BUFFER, ubo);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::vec3) * 3 + sizeof(float), NULL, GL_STATIC_DRAW); 
    }

    for (auto& shape : shapes) {        
        // since tinyobj::LoadObj() has already triangulated the mesh, so we can assert that the number of indices is a multiple of 3
        assert(shape.mesh.indices.size() % 3 == 0);
        int numFaces = shape.mesh.indices.size() / 3;

        // both of the following sizes represent the number of triangle faces
        assert(numFaces == shape.mesh.num_face_vertices.size() && numFaces == shape.mesh.material_ids.size());

        std::vector<int> faceIndices(numFaces);
        std::iota(faceIndices.begin(), faceIndices.end(), 0);

        // sort face indices by material id in order to make the faces with the same material id contiguous
        std::sort(faceIndices.begin(), faceIndices.end(), [&](int i1, int i2){
            return shape.mesh.material_ids[i1] < shape.mesh.material_ids[i2];
        });
        
        std::vector<Vertex> vertices;
        std::vector<Face> faces;
        std::map<std::tuple<int, int, int>, int> pointIndexMap;
        int materialId = shape.mesh.material_ids[0];
        for (int i = 0; i < numFaces; ++i) {
            int faceIndex = faceIndices[i];
            // new mesh
            if (materialId != shape.mesh.material_ids[faceIndex]) {
                meshes.emplace_back(std::move(vertices), std::move(faces), materialId);
                pointIndexMap.clear();
            }
            materialId = shape.mesh.material_ids[faceIndex];

            int pointIndex[3];
            for (int j = 0; j < 3; ++j) {
                int vertexIndex = shape.mesh.indices[3 * faceIndex + j].vertex_index;
                int normalIndex = shape.mesh.indices[3 * faceIndex + j].normal_index;
                int texcoordIndex = shape.mesh.indices[3 * faceIndex + j].texcoord_index;
                
                if (normalIndex == -1) {
                    throw std::runtime_error("vn data in .obj is missing");
                }
                std::tuple<int, int, int> pointIndexTuple = {vertexIndex, normalIndex, texcoordIndex};
                if (pointIndexMap.count(pointIndexTuple) == 0) {
                    pointIndexMap[pointIndexTuple] = vertices.size();
                    Vertex vertex;
                    vertex.position = {
                        attributes.vertices[3 * vertexIndex + 0],
                        attributes.vertices[3 * vertexIndex + 1],
                        attributes.vertices[3 * vertexIndex + 2]
                    };
                    vertex.normal = {
                        attributes.normals[3 * normalIndex + 0],
                        attributes.normals[3 * normalIndex + 1],
                        attributes.normals[3 * normalIndex + 2]
                    };
                    if (texcoordIndex != -1) {
                        vertex.texcoord = {
                            attributes.texcoords[2 * texcoordIndex + 0],
                            attributes.texcoords[2 * texcoordIndex + 1]
                        };
                    }
                    vertices.push_back(vertex);
                }

                pointIndex[j] = pointIndexMap[pointIndexTuple];
            }
            faces.emplace_back(pointIndex[0], pointIndex[1], pointIndex[2]);
        }
        meshes.emplace_back(std::move(vertices), std::move(faces), materialId);
    }
}
} // namespace tinyrenderer


#include "model.hpp"

#include <obj_loader/tiny_obj_loader.h>

#include <iostream>
#include <stdexcept>
#include <unordered_map>
#include <vector>

#include "utils.hpp"

namespace tinyrenderer {

namespace fs = std::filesystem;

Model::Model(Model&& other) : m_mesh(std::move(other.m_mesh)),
                              m_materials(std::move(other.m_materials)),
                              m_modelBlock(other.m_modelBlock) {
}

Model& Model::operator=(Model&& other) {
    if (this != &other) {
        m_mesh       = std::move(other.m_mesh);
        m_materials  = std::move(other.m_materials);
        m_modelBlock = other.m_modelBlock;
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
        throw std::runtime_error("Model::Model: " + err);
    }

    // 2. Create mesh from tinyobj shapes
    m_mesh = std::make_unique<Mesh>(attributes, shapes, materials.size());

    // 3. Convert tinyobj material into m_materials map
    for (auto& material : materials) {
        m_materials.push_back(std::make_shared<Material>(baseDir, material));
    }

    // 4. Initialize model transform matrix and normal matrix
    setTransformMatrix(transform);
}

Model::~Model() {
    m_mesh.reset();
    m_materials.clear();
}

void Model::getRenderQueue(std::vector<RenderItem>& queue, bool opaque) const {
    queue.clear();
    for (auto& sm : m_mesh->getSubMeshes()) {
        if (m_materials[sm.matid]->isOpaque() != opaque) {
            continue;
        }
        queue.emplace_back(
            RenderItem{
                .mesh     = m_mesh,
                .material = m_materials[sm.matid],
                .ioffset  = sm.offset,
                .length   = sm.length,
            }
        );
    }
}

}  // namespace tinyrenderer

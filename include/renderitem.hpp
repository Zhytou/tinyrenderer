#pragma once

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <memory>

#include "material.hpp"
#include "mesh.hpp"

namespace tinyglrenderer {

/**
 * @brief Lightweight, self-contained draw command block.
 * @note Acts as a granular package containing geometry handles and spatial states required
 * for a single primitive draw call. Collected dynamically into sorting queues for rendering.
 */
struct RenderItem {
    const std::shared_ptr<Mesh> mesh;
    const std::shared_ptr<Material> material;

    uint ioffset = 0;   // vertex input data offset of vbo/ibo in vertex count
    uint length  = 0;   // vertex input data length of vbo/ibo in vertex count
    float distance   = 0.f; // distance to the camera(for transparent objects sorting)
    uint uoffset = 0;   // model block index of ubo in bytes
};

} // namespace tinyglrenderer

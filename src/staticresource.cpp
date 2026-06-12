#include "staticresource.hpp"

#include <vector>

#include "mesh.hpp"

namespace tinyrenderer {

void StaticResource::initialize() {
    // 1. Define vertex layouts
    m_layouts["mesh"] = std::make_shared<VertexLayout>();
    m_layouts["mesh"]->initialize({
        VertexAttribute{
            .location   = 0,
            .size       = 3,
            .type       = GL_FLOAT,
            .offset     = offsetof(Vertex, position),
            .normalized = GL_FALSE,
            .stride     = sizeof(Vertex),
            .slot       = 0,
        },
        VertexAttribute{
            .location   = 1,
            .size       = 3,
            .type       = GL_FLOAT,
            .offset     = offsetof(Vertex, normal),
            .normalized = GL_TRUE,
            .stride     = sizeof(Vertex),
            .slot       = 0,
        },
        VertexAttribute{
            .location   = 2,
            .size       = 3,
            .type       = GL_FLOAT,
            .offset     = offsetof(Vertex, tangent),
            .normalized = GL_TRUE,
            .stride     = sizeof(Vertex),
            .slot       = 0,
        },
        VertexAttribute{
            .location   = 3,
            .size       = 2,
            .type       = GL_FLOAT,
            .offset     = offsetof(Vertex, texcoord),
            .normalized = GL_FALSE,
            .stride     = sizeof(Vertex),
            .slot       = 0,
        },
    });

    m_layouts["quad"] = std::make_shared<VertexLayout>();
    m_layouts["quad"]->initialize({
        VertexAttribute{
            .location   = 0,
            .size       = 2,
            .type       = GL_FLOAT,
            .offset     = 0,
            .normalized = GL_FALSE,
            .stride     = sizeof(float) * 4,
            .slot       = 0,
        },
        VertexAttribute{
            .location   = 1,
            .size       = 2,
            .type       = GL_FLOAT,
            .offset     = sizeof(float) * 2,
            .normalized = GL_FALSE,
            .stride     = sizeof(float) * 4,
            .slot       = 0,
        },
    });

    m_layouts["skybox"] = std::make_shared<VertexLayout>();
    m_layouts["skybox"]->initialize({
        VertexAttribute{
            .location   = 0,
            .size       = 3,
            .type       = GL_FLOAT,
            .offset     = 0,
            .normalized = GL_FALSE,
            .stride     = sizeof(float) * 3,
            .slot       = 0,
        },
    });

    // 2. Create static vertex buffers for quad and skybox
    float quad[] = {
        // Position(2f)   // TexCoords(2f)
        -1.0f, 1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 1.0f,
        1.0f, -1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f
    };
    m_buffers["quad"] = std::make_unique<VertexBuffer>(sizeof(quad), quad);

    float skybox[] = {
        // +X
        1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, -1.0f, 1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, 1.0f,
        // -X
        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, -1.0f,
        // +Y
        -1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        // -Y
        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, -1.0f,
        // +Z
        -1.0f, -1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,
        // -Z
        -1.0f, 1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, 1.0f, -1.0f
    };
    m_buffers["skybox"] = std::make_unique<VertexBuffer>(sizeof(skybox), skybox);

    // 3. Calculate vertex counts for quad and skybox
    m_counts["quad"]   = sizeof(quad) / (sizeof(float) * 4);
    m_counts["skybox"] = sizeof(skybox) / (sizeof(float) * 3);

    // 4. Attach vertex layouts to buffers for quad and skybox
    m_layouts["quad"]->attach(0, m_buffers["quad"], 0, sizeof(float) * 4);
    m_layouts["skybox"]->attach(0, m_buffers["skybox"], 0, sizeof(float) * 3);
}

void StaticResource::destroy() {
    m_layouts.clear();
    m_buffers.clear();
}

};  // namespace tinyrenderer
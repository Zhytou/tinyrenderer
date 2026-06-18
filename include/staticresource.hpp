#pragma once

#include <array>
#include <format>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include "vertexbuffer.hpp"
#include "vertexlayout.hpp"

namespace tinyrenderer {

class StaticResource {
   private:
    StaticResource()  = default;
    ~StaticResource() = default;

   public:
    StaticResource(const StaticResource&)            = delete;
    StaticResource& operator=(const StaticResource&) = delete;
    StaticResource(StaticResource&&)                 = delete;
    StaticResource& operator=(StaticResource&&)      = delete;

    static StaticResource& getInstance() {
        static StaticResource instance;
        return instance;
    }
    const GLsizei& getCounts(const std::string& name) const {
        if (!m_counts.count(name)) {
            throw std::runtime_error("StaticResource::getCounts: Vertex count for " + name + " not found in StaticResource");
        }
        return m_counts.at(name);
    }
    const std::shared_ptr<VertexLayout>& getLayout(const std::string& name) const {
        if (!m_layouts.count(name)) {
            throw std::runtime_error("StaticResource::getLayout: VertexLayout " + name + " not found in StaticResource");
        }
        return m_layouts.at(name);
    }
    const std::unique_ptr<VertexBuffer>& getBuffer(const std::string& name) const {
        if (!m_buffers.count(name)) {
            throw std::runtime_error("StaticResource::getBuffer: VertexBuffer " + name + " not found in StaticResource");
        }
        return m_buffers.at(name);
    }
    glm::mat4 getCaptureMatrix(GLint index) const {
        if (index < 0 || index >= 6) {
            throw std::runtime_error(std::format("StaticResource::getCaptureMatrix: CaptureMatrix {} not found in StaticResource", index));
        }
        return m_matrixs[index];
    }

    void
    initialize();
    void destroy();

   private:
    std::unordered_map<std::string, GLsizei> m_counts;
    std::unordered_map<std::string, std::shared_ptr<VertexLayout>> m_layouts;
    std::unordered_map<std::string, std::unique_ptr<VertexBuffer>> m_buffers;
    std::array<glm::mat4, 6> m_matrixs;
};
};  // namespace tinyrenderer

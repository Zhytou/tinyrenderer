#pragma once

#include <memory>
#include <string>
#include <vector>

#include "camera.hpp"
#include "light.hpp"
#include "model.hpp"
#include "renderitem.hpp"

namespace tinyrenderer {

class Scene {
   public:
    Scene()                        = default;
    Scene(const Scene&)            = delete;
    Scene& operator=(const Scene&) = delete;
    ~Scene() { destroy(); }
    Scene(const std::string& json) { initialize(json); }

    const std::unique_ptr<Camera>& getCamera() const { return m_camera; }
    const std::vector<std::shared_ptr<Light>>& getLights() const { return m_lights; }
    const std::vector<std::shared_ptr<Model>>& getModels() const { return m_models; }
    const std::vector<ModelBlock>& getModelBlocks() const { return m_blocks; }
    const std::pair<glm::vec3, glm::vec3>& getBoundingBox() const { return m_xyz; }
    void getRenderQueue(std::vector<RenderItem>& queue, bool opaque) const;

    void initialize(const std::string& json);
    void destroy();

   private:
    std::unique_ptr<Camera> m_camera = nullptr;
    std::vector<std::shared_ptr<Light>> m_lights;
    std::vector<std::shared_ptr<Model>> m_models;
    std::vector<ModelBlock> m_blocks;
    std::pair<glm::vec3, glm::vec3> m_xyz = {glm::vec3(0.0f), glm::vec3(0.0f)};
};

}  // namespace tinyrenderer

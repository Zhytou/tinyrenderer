#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "model.hpp"

namespace tinyrenderer
{
struct Camera {
    glm::vec3 eye;
    glm::vec3 target;
    glm::vec3 up;
};

struct Light {
	glm::vec3 position;
	glm::vec3 radiance;
};

struct Scene {
    Camera camera;
    std::vector<Light> lights;
    std::vector<Model> models;
};

} // namespace tinyrenderer
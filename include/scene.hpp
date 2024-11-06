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
    float fov;
    float aspect;
    float near;
    float far;
};

struct PointLight {
	glm::vec3 position;
	glm::vec3 color;
};

struct DirectionalLight {
    glm::vec3 direction;
	glm::vec3 color;
};

struct Scene {
    Camera camera;
    // point lights
    std::vector<PointLight> plights;
    // directional light
    DirectionalLight dlight;
    // models in the scene
    std::vector<Model> models;
    // light/camera rotation/scaling speed
    float speed;
};

} // namespace tinyrenderer
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
    float speed;
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
    std::vector<PointLight> plights;
    DirectionalLight dlight;
    std::vector<Model> models;
};

} // namespace tinyrenderer
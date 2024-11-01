#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "model.hpp"

namespace tinyrenderer
{
const unsigned maxPointLightNum = 8;

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
    PointLight plights[maxPointLightNum];
    DirectionalLight dlight;
    std::vector<Model> models;
};

} // namespace tinyrenderer
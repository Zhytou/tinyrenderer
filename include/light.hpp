#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <iostream>

#include "utils.hpp"

namespace tinyrenderer {

struct alignas(16) LightBlock {
    glm::mat4 lightSpaceMatrix;
    glm::vec4 lightColorIntensity;
    glm::vec4 lightVectorType;  // 0: directional, 1: point
};

class Light {
   public:
    Light() = default;
    Light(const glm::vec3& color, float intensity) : m_color(color), m_intensity(intensity) {}
    virtual ~Light() = default;

    const glm::vec3& getColor() const { return m_color; }
    float getIntensity() const { return m_intensity; }
    const LightBlock& getLightBlock() const { return m_lightBlock; };
    void setColor(const glm::vec3& color) {
        m_color                          = color;
        m_lightBlock.lightColorIntensity = glm::vec4(m_color, m_intensity);
    }
    void setIntensity(float intensity) {
        m_intensity                      = intensity;
        m_lightBlock.lightColorIntensity = glm::vec4(m_color, m_intensity);
    }
    virtual const glm::mat4& setLightSpaceMatrix(const std::pair<glm::vec3, glm::vec3>&) = 0;

   protected:
    LightBlock m_lightBlock;
    glm::vec3 m_color = glm::vec3(1.0f);
    float m_intensity = 1.0f;
    glm::mat4 m_lightSpaceMatrix;
};

class DirectionalLight : public Light {
   public:
    DirectionalLight() = default;
    DirectionalLight(const glm::vec3& color, float intensity, const glm::vec3& direction) : Light(color, intensity), m_direction(glm::normalize(direction)) {
        m_lightBlock.lightVectorType = glm::vec4(m_direction, 0.0f);
    }
    ~DirectionalLight() = default;

    inline const glm::mat4& setLightSpaceMatrix(const std::pair<glm::vec3, glm::vec3>& xyz) override;

   private:
    glm::vec3 m_direction;
};

inline const glm::mat4& DirectionalLight::setLightSpaceMatrix(const std::pair<glm::vec3, glm::vec3>& xyz) {
    auto& [xyz1, xyz2] = xyz;  // xyz is bounding box of the scene

    glm::vec3 sceneCenter = (xyz1 + xyz2) / 2.0f;
    glm::vec3 lightPos    = sceneCenter - m_direction * glm::length(xyz2 - xyz1) * 0.5f;
    glm::vec3 lightUp     = {0.0f, 1.0f, 0.0f};
    if (std::abs(glm::dot(m_direction, lightUp)) > 0.99f) {
        lightUp = {0.0f, 0.0f, 1.0f};
    }
    glm::mat4 lightViewMatrix = glm::lookAt(lightPos, sceneCenter, lightUp);

    glm::vec3 corners[8] = {
        {xyz1.x, xyz1.y, xyz1.z},
        {xyz2.x, xyz1.y, xyz1.z},
        {xyz1.x, xyz2.y, xyz1.z},
        {xyz2.x, xyz2.y, xyz1.z},
        {xyz1.x, xyz1.y, xyz2.z},
        {xyz2.x, xyz1.y, xyz2.z},
        {xyz1.x, xyz2.y, xyz2.z},
        {xyz2.x, xyz2.y, xyz2.z},
    };
    glm::vec3 nxyz1 = glm::vec3(FLT_MAX), nxyz2 = glm::vec3(-FLT_MAX);
    for (int i = 0; i < 8; ++i) {
        glm::vec3 ptInLightSpace = glm::vec3(lightViewMatrix * glm::vec4(corners[i], 1.0f));
        nxyz1                    = glm::min(nxyz1, ptInLightSpace);
        nxyz2                    = glm::max(nxyz2, ptInLightSpace);
    }

    glm::mat4 lightProjectionMatrix = glm::ortho(
        nxyz1.x, nxyz2.x,
        nxyz1.y, nxyz2.y,
        nxyz2.z,  // near
        nxyz1.z   // far
    );

    m_lightSpaceMatrix            = lightProjectionMatrix * lightViewMatrix;
    m_lightBlock.lightSpaceMatrix = m_lightSpaceMatrix;
    return m_lightSpaceMatrix;
}

};  // namespace tinyrenderer
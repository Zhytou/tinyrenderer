#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <iostream>

#include "utils.hpp"

namespace tinyrenderer {

struct alignas(16) LightBlock {
    glm::mat4 viewProjMatrix;
    glm::vec4 colorIntensity;
    glm::vec4 vectorType;  // .xyz vector(direction or position) .w type(0: directional or 1: point)
    glm::vec4 uvOffsetScale;
};

class Light {
   public:
    Light() = default;
    Light(const glm::vec3& color, float intensity) {
        m_lightBlock.colorIntensity = glm::vec4(color, intensity);
    }
    virtual ~Light() = default;

    glm::vec3 getColor() const { return glm::vec3(m_lightBlock.colorIntensity); }
    float getIntensity() const { return m_lightBlock.colorIntensity.w; }
    const glm::mat4& getViewProjMatrix() const { return m_lightBlock.viewProjMatrix; }
    const LightBlock& getLightBlock() const { return m_lightBlock; };
    void setColor(const glm::vec3& color) {
        m_lightBlock.colorIntensity.x = color.x;
        m_lightBlock.colorIntensity.y = color.y;
        m_lightBlock.colorIntensity.z = color.z;
    }
    void setIntensity(float intensity) {
        m_lightBlock.colorIntensity.w = intensity;
    }
    void setUVOffsetScale(const glm::vec2& offset, const glm::vec2& scale) {
        m_lightBlock.uvOffsetScale = glm::vec4(offset, scale);
    }
    virtual void setLightSpaceMatrix(const std::pair<glm::vec3, glm::vec3>&) = 0;

   protected:
    LightBlock m_lightBlock;
};

class DirectionalLight : public Light {
   public:
    DirectionalLight() = default;
    DirectionalLight(const glm::vec3& color, float intensity, const glm::vec3& direction) : Light(color, intensity) {
        m_lightBlock.vectorType = glm::vec4(glm::normalize(direction), 0.0f);
    }
    ~DirectionalLight() = default;

    glm::vec3 getDirection() const { return glm::vec3(m_lightBlock.vectorType); }
    void setDirection(const glm::vec3& direction) {
        m_lightBlock.vectorType = glm::vec4(glm::normalize(direction), 0.0f);
    }
    inline void setLightSpaceMatrix(const std::pair<glm::vec3, glm::vec3>& xyz) override;
};

inline void DirectionalLight::setLightSpaceMatrix(const std::pair<glm::vec3, glm::vec3>& xyz) {
    auto& [xyz1, xyz2] = xyz;  // xyz is bounding box of the scene

    glm::vec3 direction   = getDirection();
    glm::vec3 sceneCenter = (xyz1 + xyz2) / 2.0f;
    glm::vec3 lightPos    = sceneCenter - direction * glm::length(xyz2 - xyz1) * 0.5f;
    glm::vec3 lightUp     = {0.0f, 1.0f, 0.0f};
    if (std::abs(glm::dot(direction, lightUp)) > 0.99f) {
        lightUp = {0.0f, 0.0f, 1.0f};
    }
    glm::mat4 viewMatrix = glm::lookAt(lightPos, sceneCenter, lightUp);

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
        glm::vec3 ptInLightSpace = glm::vec3(viewMatrix * glm::vec4(corners[i], 1.0f));
        nxyz1                    = glm::min(nxyz1, ptInLightSpace);
        nxyz2                    = glm::max(nxyz2, ptInLightSpace);
    }

    glm::mat4 projMatrix = glm::ortho(
        nxyz1.x, nxyz2.x,
        nxyz1.y, nxyz2.y,
        -nxyz2.z,  // near
        -nxyz1.z   // far
    );

    m_lightBlock.viewProjMatrix = projMatrix * viewMatrix;
    return;
}

};  // namespace tinyrenderer
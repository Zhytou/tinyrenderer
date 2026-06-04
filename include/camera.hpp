#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace tinyrenderer {

struct alignas(16) CameraBlock {
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
    glm::vec3 cameraPosition;
};

class Camera {
   public:
    Camera()          = default;
    virtual ~Camera() = default;

    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    const glm::vec3& getEye() const { return m_eye; }
    const glm::vec3& getTarget() const { return m_target; }

    const glm::mat4& getViewMatrix() const { return m_viewMatrix; }
    const glm::mat4& getProjectionMatrix() const { return m_projectionMatrix; }
    const CameraBlock& getCameraBlock() const { return m_cameraBlock; }

    void setEye(glm::vec3 eye) {
        m_eye = eye;
        update();
    }
    void setTarget(glm::vec3 target) {
        m_target = target;
        update();
    }
    void setUp(glm::vec3 up) {
        m_up = up;
        update();
    }
    void setFov(float fov) {
        m_fov = fov;
        update();
    }
    void setNear(float near) {
        m_near = near;
        update();
    }
    void setFar(float far) {
        m_far = far;
        update();
    }
    void setViewport(int width, int height) {
        m_width  = width;
        m_height = height;
        m_aspect = static_cast<float>(width) / static_cast<float>(height);
        update();
    }

   protected:
    virtual void update() = 0;

    CameraBlock m_cameraBlock;
    glm::mat4 m_viewMatrix;
    glm::mat4 m_projectionMatrix;
    uint32_t m_dirty = 0;
    glm::vec3 m_eye;
    glm::vec3 m_target;
    glm::vec3 m_up;

    float m_fov;
    float m_near;
    float m_far;
    float m_aspect;
    int m_width;
    int m_height;
};

class PerspectiveCamera : public Camera {
   public:
    PerspectiveCamera()  = default;
    ~PerspectiveCamera() = default;

   protected:
    void update() override {
        m_projectionMatrix             = glm::perspective(glm::radians(m_fov), m_aspect, m_near, m_far);
        m_viewMatrix                   = glm::lookAt(m_eye, m_target, m_up);
        m_cameraBlock.viewMatrix       = m_viewMatrix;
        m_cameraBlock.projectionMatrix = m_projectionMatrix;
        m_cameraBlock.cameraPosition   = m_eye;
    }
};

class OrthographicCamera : public Camera {
   public:
    OrthographicCamera()  = default;
    ~OrthographicCamera() = default;

   protected:
    void update() override {
        m_projectionMatrix             = glm::ortho(-m_width / 2.0f, m_width / 2.0f, -m_height / 2.0f, m_height / 2.0f, m_near, m_far);
        m_viewMatrix                   = glm::lookAt(m_eye, m_target, m_up);
        m_cameraBlock.viewMatrix       = m_viewMatrix;
        m_cameraBlock.projectionMatrix = m_projectionMatrix;
        m_cameraBlock.cameraPosition   = m_eye;
    }
};

}  // namespace tinyrenderer
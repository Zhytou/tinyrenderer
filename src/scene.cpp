#include "scene.hpp"

#include <rapidjson/document.h>

namespace tinyrenderer {

void Scene::getRenderQueue(std::vector<RenderItem>& queue, bool opaque) const {
    for (int i = 0; i < m_models.size(); i++) {
        std::vector<RenderItem> subQueue;
        m_models[i]->getRenderQueue(subQueue, opaque);
        auto xyz       = m_models[i]->getBoundingBox();
        float distance = m_camera->getDistance((xyz.first + xyz.second) / 2.f);
        for (auto& item : subQueue) {
            item.distance = distance;
            item.uoffset  = i;
            queue.push_back(item);
        }
    }
}

void Scene::initialize(const std::string& json) {
    rapidjson::Document doc;
    if (doc.Parse(json.c_str()).HasParseError()) {
        throw std::runtime_error("Error parsing JSON");
    }
    auto getVec3 = [](rapidjson::Value& arr) -> glm::vec3 {
        if (!arr.IsArray() || arr.Size() != 3 || !arr[0].IsNumber() || !arr[1].IsNumber() || !arr[2].IsNumber()) {
            throw std::runtime_error("Invalid array size or element type");
        }

        return glm::vec3{arr[0].GetFloat(), arr[1].GetFloat(), arr[2].GetFloat()};
    };

    // lights
    if (doc.HasMember("lights")) {
        for (int i = 0; i < doc["lights"]["directionallight"].Size(); i++) {
            m_lights.emplace_back(std::make_shared<DirectionalLight>(getVec3(doc["lights"]["directionallight"][i]["color"]), doc["lights"]["directionallight"][i]["intensity"].GetFloat(), getVec3(doc["lights"]["directionallight"][i]["direction"])));
        }
    }

    // models
    if (doc.HasMember("models")) {
        for (int i = 0; i < doc["models"].Size(); i++) {
            std::string baseDir   = doc["models"][i]["baseDir"].GetString();
            std::string modelName = doc["models"][i]["name"].GetString();
            glm::vec3 translate   = doc["models"][i]["transform"].HasMember("translate") ? getVec3(doc["models"][i]["transform"]["translate"]) : glm::vec3(0.0f);
            glm::vec3 rotate      = doc["models"][i]["transform"].HasMember("rotate") ? getVec3(doc["models"][i]["transform"]["rotate"]) : glm::vec3(0.0f);
            glm::vec3 scale       = doc["models"][i]["transform"].HasMember("scale") ? getVec3(doc["models"][i]["transform"]["scale"]) : glm::vec3(1.0f);

            glm::mat4 transform = glm::mat4(1.0f);
            transform           = glm::translate(transform, translate);
            transform           = glm::rotate(transform, glm::radians(rotate.x), glm::vec3(1.0f, 0.0f, 0.0f));
            transform           = glm::rotate(transform, glm::radians(rotate.y), glm::vec3(0.0f, 1.0f, 0.0f));
            transform           = glm::rotate(transform, glm::radians(rotate.z), glm::vec3(0.0f, 0.0f, 1.0f));
            transform           = glm::scale(transform, scale);
            m_models.emplace_back(std::make_shared<Model>(baseDir, modelName, transform));

            auto [xyzi1, xyzi2] = m_models.back()->getBoundingBox();
            m_xyz.first         = glm::min(m_xyz.first, xyzi1);
            m_xyz.second        = glm::max(m_xyz.second, xyzi2);
            m_blocks.emplace_back(m_models.back()->getModelBlock());
        }
    }

    // camera
    if (doc.HasMember("camera")) {
        m_camera = std::make_unique<PerspectiveCamera>();
        m_camera->setEye(getVec3(doc["camera"]["eye"]));
        m_camera->setTarget(getVec3(doc["camera"]["target"]));
        m_camera->setUp(getVec3(doc["camera"]["up"]));
        m_camera->setFov(doc["camera"]["fov"].GetFloat());
        m_camera->setNear(doc["camera"]["near"].GetFloat());
        m_camera->setFar(doc["camera"]["far"].GetFloat());
        m_camera->setViewport(doc["camera"]["width"].GetInt(), doc["camera"]["height"].GetInt());
    } else {
        const glm::vec3 xyz1 = m_xyz.first, xyz2 = m_xyz.second;
        glm::vec3 target    = 0.5f * (xyz2 + xyz1);
        glm::vec3 eye       = target + 1.0f * (xyz2 - xyz1);
        glm::vec3 direction = glm::normalize(xyz2 - xyz1);
        glm::vec3 up        = {0.0f, 1.0f, 0.0f};
        if (std::abs(glm::dot(direction, up)) > 0.99f) {
            up = {0.0f, 0.0f, 1.0f};
        }
        m_camera = std::make_unique<PerspectiveCamera>();
        m_camera->setEye(eye);
        m_camera->setTarget(target);
        m_camera->setUp(up);
    }

    // TODO: add environment map (IBL) support
}

void Scene::destroy() {
    m_camera.reset();
    m_lights.clear();
    m_models.clear();
    m_blocks.clear();
}

}  // namespace tinyrenderer
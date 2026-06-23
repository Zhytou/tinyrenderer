#include "scene.hpp"

#include <rapidjson/document.h>

#include <filesystem>

#include "utils.hpp"

namespace tinyglrenderer {

namespace fs = std::filesystem;

void Scene::getLightBlocks(std::vector<LightBlock>& blocks) const {
    blocks.clear();
    for (auto& light : m_lights) {
        blocks.emplace_back(light->getLightBlock());
    }
}

void Scene::getModelBlocks(std::vector<ModelBlock>& blocks) const {
    blocks.clear();
    for (auto& model : m_models) {
        blocks.emplace_back(model->getModelBlock());
    }
}

void Scene::getRenderQueue(std::vector<RenderItem>& queue, bool opaque) const {
    queue.clear();
    for (int i = 0; i < m_models.size(); i++) {
        std::vector<RenderItem> subQueue;
        m_models[i]->getRenderQueue(subQueue, opaque);
        auto xyz       = m_models[i]->getBoundingBox();
        float distance = m_camera->getDistance((xyz.first + xyz.second) / 2.f);
        for (auto& item : subQueue) {
            item.distance = distance;
            item.uoffset  = i * sizeof(ModelBlock);
            queue.push_back(item);
        }
    }
}

void Scene::initialize(const std::string& json) {
    rapidjson::Document doc;
    if (doc.Parse(json.c_str()).HasParseError()) {
        throw std::runtime_error("Scene::initialize: Error parsing JSON");
    }
    auto getVec3 = [](rapidjson::Value& arr) -> glm::vec3 {
        if (!arr.IsArray() || arr.Size() != 3 || !arr[0].IsNumber() || !arr[1].IsNumber() || !arr[2].IsNumber()) {
            throw std::runtime_error("Scene::initialize: Invalid array size or element type");
        }

        return glm::vec3{arr[0].GetFloat(), arr[1].GetFloat(), arr[2].GetFloat()};
    };

    // models
    if (doc.HasMember("models")) {
        for (int i = 0; i < doc["models"].Size(); i++) {
            fs::path baseDir    = doc["models"][i]["baseDir"].GetString();
            fs::path modelName  = doc["models"][i]["name"].GetString();
            glm::vec3 translate = doc["models"][i]["transform"].HasMember("translate") ? getVec3(doc["models"][i]["transform"]["translate"]) : glm::vec3(0.0f);
            glm::vec3 rotate    = doc["models"][i]["transform"].HasMember("rotate") ? getVec3(doc["models"][i]["transform"]["rotate"]) : glm::vec3(0.0f);
            glm::vec3 scale     = doc["models"][i]["transform"].HasMember("scale") ? getVec3(doc["models"][i]["transform"]["scale"]) : glm::vec3(1.0f);

            glm::mat4 transform = glm::mat4(1.0f);
            transform           = glm::translate(transform, translate);
            transform           = glm::rotate(transform, glm::radians(rotate.x), glm::vec3(1.0f, 0.0f, 0.0f));
            transform           = glm::rotate(transform, glm::radians(rotate.y), glm::vec3(0.0f, 1.0f, 0.0f));
            transform           = glm::rotate(transform, glm::radians(rotate.z), glm::vec3(0.0f, 0.0f, 1.0f));
            transform           = glm::scale(transform, scale);
            m_models.emplace_back(std::make_shared<Model>(baseDir, modelName, transform));

            auto [xyzi1, xyzi2] = m_models.back()->getBoundingBox();
            m_bounds.first      = glm::min(m_bounds.first, xyzi1);
            m_bounds.second     = glm::max(m_bounds.second, xyzi2);
            // std::cout << "Model BoundingBox: [" << xyzi1 << ", " << xyzi2 << "]\n";
        }
    }

    // lights
    if (doc.HasMember("lights")) {
        for (int i = 0; i < doc["lights"]["directionallight"].Size(); i++) {
            m_lights.emplace_back(std::make_shared<DirectionalLight>(getVec3(doc["lights"]["directionallight"][i]["color"]), doc["lights"]["directionallight"][i]["intensity"].GetFloat(), getVec3(doc["lights"]["directionallight"][i]["direction"])));
            m_lights.back()->setLightSpaceMatrix(m_bounds);  // set light space matrix
        }
    }

    // camera
    if (doc.HasMember("camera")) {
        m_camera = std::make_shared<PerspectiveCamera>();
        m_camera->setEye(getVec3(doc["camera"]["eye"]));
        m_camera->setTarget(getVec3(doc["camera"]["target"]));
        m_camera->setUp(getVec3(doc["camera"]["up"]));
        m_camera->setFov(doc["camera"]["fov"].GetFloat());
        m_camera->setNear(doc["camera"]["near"].GetFloat());
        m_camera->setFar(doc["camera"]["far"].GetFloat());
        m_camera->setViewport(doc["camera"]["width"].GetInt(), doc["camera"]["height"].GetInt());
        m_camera->setSpeed(doc["camera"]["speed"].GetFloat());
    } else {
        const glm::vec3 xyz1 = m_bounds.first, xyz2 = m_bounds.second;
        glm::vec3 target    = 0.5f * (xyz2 + xyz1);
        glm::vec3 eye       = target + 1.0f * (xyz2 - xyz1);
        glm::vec3 direction = glm::normalize(xyz2 - xyz1);
        glm::vec3 up        = {0.0f, 1.0f, 0.0f};
        if (std::abs(glm::dot(direction, up)) > 0.99f) {
            up = {0.0f, 0.0f, 1.0f};
        }
        m_camera = std::make_shared<PerspectiveCamera>();
        m_camera->setEye(eye);
        m_camera->setTarget(target);
        m_camera->setUp(up);
    }

    // skybox(environment map)
    if (doc.HasMember("skybox")) {
        if (doc["skybox"].HasMember("cubemap")) {
            std::vector<std::shared_ptr<Image>> images;
            fs::path baseDir = doc["skybox"]["cubemap"]["baseDir"].GetString();
            uint32_t width = 0, height = 0;
            //! WARNING: Face order must be: right, left, top, bottom, front, back.
            //! This sequence perfectly aligns with OpenGL DSA texture array layer mapping (0 to 5).
            for (auto face : {"right", "left", "top", "bottom", "front", "back"}) {
                std::string imageName = doc["skybox"]["cubemap"][face].GetString();
                auto image            = Image::create(baseDir / imageName);
                if (width == 0 && height == 0) {
                    width  = image->getWidth();
                    height = image->getHeight();
                } else if (width != image->getWidth() || height != image->getHeight()) {
                    throw std::runtime_error("Scene::initialize: Cubemap face " + std::string(face) + " has different size from other faces.");
                }
                images.push_back(image);
            }

            m_skyboxCubemap = std::make_shared<Texture>(width, height, GL_TEXTURE_CUBE_MAP, GL_RGBA32F, 1);
            for (GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X; face <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z; face++) {
                GLint index = face - GL_TEXTURE_CUBE_MAP_POSITIVE_X;
                m_skyboxCubemap->upload(images[index], index, 0);
            }
        }
        if (doc["skybox"].HasMember("equirect")) {
            fs::path baseDir      = doc["skybox"]["equirect"]["baseDir"].GetString();
            fs::path equirectName = doc["skybox"]["equirect"]["name"].GetString();
            auto image            = Image::create(baseDir / equirectName, 0, true);
            m_skyboxEquirect      = std::make_shared<Texture>(image->getWidth(), image->getHeight(), GL_TEXTURE_2D, GL_RGBA32F, 1);
            m_skyboxEquirect->upload(image);
        }
    }
}

void Scene::destroy() {
    m_skyboxCubemap.reset();
    m_skyboxEquirect.reset();
    m_camera.reset();
    m_lights.clear();
    m_models.clear();
}

}  // namespace tinyglrenderer
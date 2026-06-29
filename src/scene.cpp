#include "scene.hpp"

#include <rapidjson/document.h>

#include <filesystem>

#include "utils.hpp"

namespace tinyglrenderer {

namespace fs = std::filesystem;

void Scene::getLightBlocks(std::vector<LightBlock>& blocks) const {
    blocks.clear();
    for (auto& light : m_lights) { blocks.emplace_back(light->getLightBlock()); }
}

void Scene::getModelBlocks(std::vector<ModelBlock>& blocks) const {
    blocks.clear();
    for (auto& model : m_models) { blocks.emplace_back(model->getModelBlock()); }
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

void Scene::initialize(const std::string& json, ResourceManager& manager) {
    rapidjson::Document doc;
    if (doc.Parse(json.c_str()).HasParseError()) { throw std::runtime_error("Scene::initialize: Error parsing JSON"); }
    auto getVec3 = [](rapidjson::Value& arr) -> glm::vec3 {
        if (!arr.IsArray() || arr.Size() != 3 || !arr[0].IsNumber() || !arr[1].IsNumber() || !arr[2].IsNumber()) { throw std::runtime_error("Scene::initialize: Invalid array size or element type"); }

        return glm::vec3{arr[0].GetFloat(), arr[1].GetFloat(), arr[2].GetFloat()};
    };

    // models
    if (doc.HasMember("models")) {
        for (int i = 0; i < doc["models"].Size(); i++) {
            auto& modelDoc = doc["models"][i];
            // model base dir and name required
            fs::path baseDir   = modelDoc["base_dir"].GetString();
            fs::path modelName = modelDoc["name"].GetString();
            m_models.emplace_back(manager.loadModel(baseDir, modelName));

            // default material optional
            std::map<std::vector<const char*>, const char*> mat2Type = {
                {{"albedo"}, "vec3"},
                {{"normal"}, "vec3"},
                {{"metallic", "roughness", "ambient"}, "float"},
                {{"albedo_map"}, "string"},
                {{"normal_map"}, "string"},
                {{"metallic_map", "roughness_map", "ambient_map"}, "string"},
            };
            if (modelDoc.HasMember("default_mat")) {
                auto& matDoc = modelDoc["default_mat"];
                glm::vec3 albedo = matDoc.HasMember("albedo") ? getVec3(matDoc["albedo"]) : glm::vec3(0.5f);
                float metallic = matDoc.HasMember("metallic") ? matDoc["metallic"].GetFloat() : 0.0f;
                float roughness = matDoc.HasMember("roughness") ? matDoc["roughness"].GetFloat() : 0.0f;

                std::string baseMatDir = matDoc.HasMember("base_dir") ? matDoc["base_dir"].GetString() : "";
                tinyobj::material_t material;
                material.name = matDoc.HasMember("name") ? matDoc["name"].GetString() : modelName.string() + "_default";
                material.diffuse[0] = albedo[0];
                material.diffuse[1] = albedo[1];
                material.diffuse[2] = albedo[2];
                material.metallic = metallic;
                material.roughness = roughness;
                material.diffuse_texname = matDoc.HasMember("albedo_map") ? matDoc["albedo_map"].GetString() : "";
                material.normal_texname = matDoc.HasMember("normal_map") ? matDoc["normal_map"].GetString() : "";
                material.metallic_texname = matDoc.HasMember("metallic_map") ? matDoc["metallic_map"].GetString() : "";
                material.roughness_texname = matDoc.HasMember("roughness_map") ? matDoc["roughness_map"].GetString() : "";
                material.ambient_texname = matDoc.HasMember("ambient_map") ? matDoc["ambient_map"].GetString() : "";

                auto nmaterial = manager.loadMaterial(baseMatDir, material);
                m_models.back()->setDefaultMaterial(nmaterial);
            }

            glm::vec3 translate = glm::vec3(0.0f);
            glm::vec3 rotate    = glm::vec3(0.0f);
            glm::vec3 scale     = glm::vec3(1.0f);
            if (doc["models"][i].HasMember("transform")) {
                translate = modelDoc["transform"].HasMember("translate") ? getVec3(modelDoc["transform"]["translate"]) : glm::vec3(0.0f);
                rotate    = modelDoc["transform"].HasMember("rotate") ? getVec3(modelDoc["transform"]["rotate"]) : glm::vec3(0.0f);
                scale     = modelDoc["transform"].HasMember("scale") ? getVec3(modelDoc["transform"]["scale"]) : glm::vec3(1.0f);
            }
            m_models.back()->setTransform(translate, rotate, scale);

            auto [xyzi1, xyzi2] = m_models.back()->getBoundingBox();
            m_bounds.first      = glm::min(m_bounds.first, xyzi1);
            m_bounds.second     = glm::max(m_bounds.second, xyzi2);
        }
    }

        // skybox(environment map)
    if (doc.HasMember("skybox")) {
        if (doc["skybox"].HasMember("cubemap")) {
            std::vector<fs::path> imagePaths;
            fs::path baseDir = doc["skybox"]["cubemap"]["base_dir"].GetString();
            for (auto face : {"right", "left", "top", "bottom", "front", "back"}) { // face order must be: right, left, top, bottom, front, back.
                std::string imageName = doc["skybox"]["cubemap"][face].GetString();
                imagePaths.push_back(baseDir / imageName);
            }
            m_skyboxCubemap = manager.loadCubeTexture(imagePaths, glm::vec4(0.0f), GL_RGBA32F, 1);
        }
        if (doc["skybox"].HasMember("equirect")) {
            fs::path baseDir      = doc["skybox"]["equirect"]["base_dir"].GetString();
            fs::path equirectName = doc["skybox"]["equirect"]["name"].GetString();
            m_skyboxEquirect      = manager.load2DTexture(baseDir / equirectName, glm::vec4(0.0f), GL_RGBA32F, 1);
        }
    }

    // lights
    if (doc.HasMember("lights")) {
        for (int i = 0; i < doc["lights"]["directional"].Size(); i++) {
            auto& lightDoc = doc["lights"]["directional"][i];
            m_lights.emplace_back(std::make_shared<DirectionalLight>(getVec3(lightDoc["color"]), lightDoc["intensity"].GetFloat(), getVec3(lightDoc["direction"])));
            m_lights.back()->setLightSpaceMatrix(m_bounds); // set light space matrix
        }
    }

    // camera
    if (doc.HasMember("camera")) {
        auto& cameraDoc = doc["camera"];
        m_camera        = std::make_shared<PerspectiveCamera>();
        m_camera->setEye(getVec3(cameraDoc["eye"]));
        m_camera->setTarget(getVec3(cameraDoc["target"]));
        m_camera->setUp(getVec3(cameraDoc["up"]));
        m_camera->setFov(cameraDoc["fov"].GetFloat());
        m_camera->setNear(cameraDoc["near"].GetFloat());
        m_camera->setFar(cameraDoc["far"].GetFloat());
        m_camera->setAspect(cameraDoc["width"].GetInt(), cameraDoc["height"].GetInt());
        m_camera->setSpeed(cameraDoc["speed"].GetFloat());
    } else {
        const glm::vec3 xyz1 = m_bounds.first, xyz2 = m_bounds.second;
        glm::vec3 target    = 0.5f * (xyz2 + xyz1);
        glm::vec3 eye       = target + 1.0f * (xyz2 - xyz1);
        glm::vec3 direction = glm::normalize(xyz2 - xyz1);
        glm::vec3 up        = {0.0f, 1.0f, 0.0f};
        if (std::abs(glm::dot(direction, up)) > 0.99f) { up = {0.0f, 0.0f, 1.0f}; }
        m_camera = std::make_shared<PerspectiveCamera>();
        m_camera->setEye(eye);
        m_camera->setTarget(target);
        m_camera->setUp(up);
    }
}

void Scene::destroy() {
    m_skyboxCubemap.reset();
    m_skyboxEquirect.reset();
    m_camera.reset();
    m_lights.clear();
    m_models.clear();
}

} // namespace tinyglrenderer
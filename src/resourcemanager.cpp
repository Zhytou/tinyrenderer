#include "resourcemanager.hpp"

#include <vector>

#include "utils.hpp"

namespace tinyglrenderer {

namespace fs = std::filesystem;

std::unordered_map<std::string, GLsizei> ResourceManager::m_counts;
std::unordered_map<std::string, std::shared_ptr<VertexLayout>> ResourceManager::m_layouts;
std::unordered_map<std::string, std::unique_ptr<VertexBuffer>> ResourceManager::m_buffers;
std::array<glm::mat4, 6> ResourceManager::m_matrixs;

void ResourceManager::initialize() {
    // 1. Define vertex layouts
    m_layouts["mesh"] = std::make_shared<VertexLayout>();
    m_layouts["mesh"]->initialize({
        VertexAttribute{
            .location   = 0,
            .size       = 3,
            .type       = GL_FLOAT,
            .offset     = offsetof(Vertex, position),
            .normalized = GL_FALSE,
            .stride     = sizeof(Vertex),
            .slot       = 0,
        },
        VertexAttribute{
            .location   = 1,
            .size       = 3,
            .type       = GL_FLOAT,
            .offset     = offsetof(Vertex, normal),
            .normalized = GL_TRUE,
            .stride     = sizeof(Vertex),
            .slot       = 0,
        },
        VertexAttribute{
            .location   = 2,
            .size       = 3,
            .type       = GL_FLOAT,
            .offset     = offsetof(Vertex, tangent),
            .normalized = GL_TRUE,
            .stride     = sizeof(Vertex),
            .slot       = 0,
        },
        VertexAttribute{
            .location   = 3,
            .size       = 2,
            .type       = GL_FLOAT,
            .offset     = offsetof(Vertex, texcoord),
            .normalized = GL_FALSE,
            .stride     = sizeof(Vertex),
            .slot       = 0,
        },
    });

    m_layouts["quad"] = std::make_shared<VertexLayout>();
    m_layouts["quad"]->initialize({
        VertexAttribute{
            .location   = 0,
            .size       = 2,
            .type       = GL_FLOAT,
            .offset     = 0,
            .normalized = GL_FALSE,
            .stride     = sizeof(float) * 4,
            .slot       = 0,
        },
        VertexAttribute{
            .location   = 1,
            .size       = 2,
            .type       = GL_FLOAT,
            .offset     = sizeof(float) * 2,
            .normalized = GL_FALSE,
            .stride     = sizeof(float) * 4,
            .slot       = 0,
        },
    });

    m_layouts["cube"] = std::make_shared<VertexLayout>();
    m_layouts["cube"]->initialize({
        VertexAttribute{
            .location   = 0,
            .size       = 3,
            .type       = GL_FLOAT,
            .offset     = 0,
            .normalized = GL_FALSE,
            .stride     = sizeof(float) * 3,
            .slot       = 0,
        },
    });

    // 2. Create static vertex buffers for quad and cube
    float quad[][4] = {
        // Position(2f) and TexCoords(2f)
        {-1.0f, 1.0f, 0.0f, 1.0f},
        {-1.0f, -1.0f, 0.0f, 0.0f},
        {1.0f, -1.0f, 1.0f, 0.0f},
        {-1.0f, 1.0f, 0.0f, 1.0f},
        {1.0f, -1.0f, 1.0f, 0.0f},
        {1.0f, 1.0f, 1.0f, 1.0f},
    };
    m_buffers["quad"] = std::make_unique<VertexBuffer>(sizeof(quad), quad);

    float cube[][18] = {
        // +X Position(3f) * 6
        {1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f},
        // -X
        {-1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f},
        // +Y
        {-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f},
        // -Y
        {-1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f},
        // +Z
        {-1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f},
        // -Z
        {-1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f},
    };
    m_buffers["cube"] = std::make_unique<VertexBuffer>(sizeof(cube), cube);

    // 3. Calculate vertex counts for quad and cube
    m_counts["quad"] = sizeof(quad) / (sizeof(float) * 4);
    m_counts["cube"] = sizeof(cube) / (sizeof(float) * 3);

    // 4. Attach vertex layouts to buffers for quad and cube
    m_layouts["quad"]->attach(0, m_buffers["quad"], 0, sizeof(float) * 4);
    m_layouts["cube"]->attach(0, m_buffers["cube"], 0, sizeof(float) * 3);

    // 5. Initialize view projection matrices
    glm::mat4 projMatrix    = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 viewMatrixs[] = {glm::lookAt(glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)), glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)), glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)), glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)), glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)), glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))};
    for (GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X; face <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z; face++) {
        int index        = face - GL_TEXTURE_CUBE_MAP_POSITIVE_X;
        m_matrixs[index] = projMatrix * viewMatrixs[index];
    }
}

void ResourceManager::destroy() {
    m_layouts.clear();
    m_buffers.clear();
    m_materials.clear();
    m_meshes.clear();
    m_textures.clear();
    m_images.clear();
}

std::shared_ptr<Model> ResourceManager::loadModel(const fs::path& baseDir, const fs::path& modelName) {
    // 1. Load obj model with tinyobj loader
    fs::path modelPath = baseDir / modelName;
    tinyobj::attrib_t attributes;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string err;
    if (!tinyobj::LoadObj(&attributes, &shapes, &materials, &err, modelPath.c_str(), baseDir.c_str(), true)) {
        throw std::runtime_error("ResourceManager::loadModel: " + err);
    }

    // 2. Create mesh from tinyobj shapes
    std::cout << "Loading mesh [" << baseDir / modelName << "]\n";
    std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>(attributes, shapes, materials.size());
    m_meshes[modelName.string()] = mesh; // add mesh to cache

    // 3. Convert tinyobj material into self-defined material
    std::cout << "Loading materials [";
    std::vector<std::shared_ptr<Material>> nmaterials;
    for (auto& material : materials) {
        std::cout << material.name << ", ";
        nmaterials.push_back(loadMaterial(baseDir, material));
    }
    std::cout << "]\n";

    return make_shared<Model>(mesh, nmaterials);
}


std::shared_ptr<Material> ResourceManager::loadMaterial(const fs::path& baseDir, const tinyobj::material_t& material) {
    glm::vec4 albedo = glm::vec4(material.diffuse[0], material.diffuse[1], material.diffuse[2], 1.0f);
    glm::vec4 normal = glm::vec4(0.5f, 0.5f, 1.0f, 0.f);
    glm::vec4 mrao = glm::vec4(material.metallic, material.roughness, 0.f, 0.f);

    std::unordered_map<std::string, std::shared_ptr<Texture>> textures = {
        // TODO: fix mip level(when miplevel is more than 1, the render result is wrong, blocking artifacts appear)
        {"albedo", load2DTexture(baseDir / material.diffuse_texname, albedo)},   
        {"normal", load2DTexture(baseDir / material.normal_texname, normal)}, 
        {"mrao", load2DTexture({baseDir / material.metallic_texname, baseDir / material.roughness_texname, baseDir / material.ambient_texname}, mrao)}
    };
    auto nmaterial = std::make_shared<Material>(material.name, textures);
    m_materials[material.name] = nmaterial;

    return nmaterial;
}

std::shared_ptr<Texture> ResourceManager::load2DTexture(const fs::path& texPath, const glm::vec4& defaultValue, GLenum internalFormat, GLsizei mipLevels) {
    std::shared_ptr<Texture> texture;
    if (fs::is_regular_file(texPath)) {
        std::cout << "Loading texture(GL_TEXTURE_2D) from file [" << texPath << "]\n";
        std::shared_ptr<Image> image  = loadImage(texPath, 0, true);
        texture = std::make_shared<Texture>(image->getWidth(), image->getHeight(), GL_TEXTURE_2D, internalFormat, mipLevels);
        texture->upload(image);
    } else {
        std::cout << "Loading texture(GL_TEXTURE_2D) from value [" << defaultValue << "]\n";
        texture = std::make_shared<Texture>(1, 1, GL_TEXTURE_2D, internalFormat, 1);
        texture->clear(glm::value_ptr(defaultValue), GL_RGBA, GL_FLOAT); // GL_RGBA and GL_FLOAT indicate the format of defaultValue is RGBA float
    }
    // m_textures[texPath.string()] = texture;
    return texture;
}

bool is_all_regular_file(const std::vector<fs::path>& paths) {
    for (auto& path : paths) {
        if (!fs::is_regular_file(path)) {
            return false;
        }
    }
    return true;
}

std::shared_ptr<Texture> ResourceManager::load2DTexture(const std::vector<fs::path>& texPaths, const glm::vec4& defaultValue, GLenum internalFormat, GLsizei mipLevels) {
    std::shared_ptr<Texture> texture;
    if (is_all_regular_file(texPaths)) {
        std::vector<std::shared_ptr<Image>> images;
        std::cout << "Loading texture(GL_TEXTURE_2D) from file [";
        for (auto& texPath : texPaths) {
            std::cout << texPath << ", ";
            images.push_back(loadImage(texPath, 1, true));
        }
        std::cout << "]\n";
        auto mimage = Image::merge(images, 3);
        texture = std::make_shared<Texture>(mimage->getWidth(), mimage->getHeight(), GL_TEXTURE_2D, GL_RGBA8, mipLevels);
        texture->upload(mimage);
    } else {
        std::cout << "Loading texture(GL_TEXTURE_2D) from value [" << defaultValue << "]\n";
        texture = std::make_shared<Texture>(1, 1, GL_TEXTURE_2D, internalFormat, 1);
        texture->clear(glm::value_ptr(defaultValue), GL_RGBA, GL_FLOAT); // GL_RGBA and GL_FLOAT indicate the format of defaultValue is RGBA float
    }
    return texture;
}

std::shared_ptr<Texture> ResourceManager::loadCubeTexture(const std::vector<fs::path>& texPaths, const glm::vec4& defaultValue, GLenum internalFormat, GLsizei mipLevels) {
    std::vector<std::shared_ptr<Image>> images;
    GLsizei width = 1, height = 1;
    if (is_all_regular_file(texPaths)) {
        std::cout << "Loading texture(GL_TEXTURE_CUBE_MAP) from file [";
        for (auto& texPath : texPaths) {
            std::cout << texPath << ", ";
            images.push_back(loadImage(texPath, 1, true));
            width  = images.back()->getWidth();
            height = images.back()->getHeight();
        }
        std::cout << "]\n";
    } else {
        std::cout << "Loading texture(GL_TEXTURE_CUBE_MAP) from value [" << defaultValue << "]\n";
    }

    std::shared_ptr<Texture> texture = std::make_shared<Texture>(width, height, GL_TEXTURE_CUBE_MAP, internalFormat, mipLevels);
    for (auto face = GL_TEXTURE_CUBE_MAP_POSITIVE_X; face <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z; face++) {
        GLint index = face - GL_TEXTURE_CUBE_MAP_POSITIVE_X;
        if (index < images.size() && width == images[index]->getWidth() && height == images[index]->getHeight()) {
            texture->upload(images[index], index, 0);
        } else {
            texture->clear(glm::value_ptr(defaultValue), GL_RGBA, GL_FLOAT); // GL_RGBA and GL_FLOAT indicate the format of defaultValue is RGBA float
        }
    }
    return texture;
}

std::shared_ptr<Image> ResourceManager::loadImage(const fs::path& imagePath, int desiredChannels, bool verticalFlip) {
    auto image = Image::create(imagePath, desiredChannels, verticalFlip); // just use static constructor Image::create()
    m_images[imagePath.string()] = image;
    return image;
}

std::shared_ptr<Shader> ResourceManager::loadShader(const fs::path& vertexShaderPath, const fs::path& fragmentShaderPath) {
    std::shared_ptr<Shader> shader = std::make_shared<Shader>(vertexShaderPath, fragmentShaderPath);
    m_shaders[vertexShaderPath.string()] = shader;
    return shader;
}

}; // namespace tinyglrenderer
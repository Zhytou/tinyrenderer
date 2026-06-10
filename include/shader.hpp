#pragma once

#include <glad/glad.h>

#include <filesystem>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace tinyrenderer {

class Shader {
   public:
    Shader() = default;
    Shader(const std::filesystem::path&, const std::filesystem::path&);
    Shader(const Shader&)            = delete;
    Shader& operator=(const Shader&) = delete;
    Shader(Shader&& other) noexcept : m_id(other.m_id), m_locations(std::move(other.m_locations)) {
        other.m_id = 0;
    }
    ~Shader();

    uint32_t getUniformLocation(const std::string& name);
    template <typename T>
    void setUniformValue(const std::string& name, const T& value);
    // void setUniformBinding(uint32_t binding);

    void use() const;

   private:
    GLuint m_id = 0;
    std::unordered_map<std::string, uint32_t> m_locations;   // normal uniform variable locations
    std::unordered_map<std::string, uint32_t> m_bindings;    // uniform block/teture slot binding points
    std::unordered_map<std::string, uint32_t> m_attributes;  // attribute binding points

    static std::string precompile(const std::filesystem::path& filename);
    static GLuint compile(const std::string& source, GLenum type);
    static GLuint link(const std::initializer_list<GLuint>& shaders);
    void introspect();
};

template <typename T>
void Shader::setUniformValue(const std::string& name, const T& value) {
    auto loc = getUniformLocation(name);
    if constexpr (std::is_same_v<T, bool>) {
        glUniform1i(loc, static_cast<int>(value));
    } else if constexpr (std::is_same_v<T, int>) {
        glUniform1i(loc, value);
    } else if constexpr (std::is_same_v<T, float>) {
        glUniform1f(loc, value);
    } else if constexpr (std::is_same_v<T, glm::vec2>) {
        glUniform2fv(loc, 1, glm::value_ptr(value));
    } else if constexpr (std::is_same_v<T, glm::vec3>) {
        glUniform3fv(loc, 1, glm::value_ptr(value));
    } else if constexpr (std::is_same_v<T, glm::mat4>) {
        glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(value));
    } else {
        throw std::runtime_error("Shader uniform valuable set failed: " + name);
    }
}

}  // namespace tinyrenderer

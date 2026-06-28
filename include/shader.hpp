#pragma once

#include <glad/glad.h>

#include <filesystem>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <unordered_map>
#include <vector>

namespace tinyglrenderer {

class Shader {
   public:
    Shader() = default;
    Shader(const std::filesystem::path&, const std::filesystem::path&);
    Shader(const Shader&)            = delete;
    Shader& operator=(const Shader&) = delete;
    ~Shader();

    GLint getUniformLocation(const std::string& name);
    template <typename T> void setUniformValue(const std::string& name, const T& value);

    void use() const;

   private:
    GLuint m_id = 0;
    std::pair<std::string, std::string> m_filename;
    std::pair<std::string, std::string> m_source;
    std::unordered_map<std::string, GLint> m_locations; // normal uniform variable locations
    std::unordered_map<std::string, GLint> m_bindings;  // uniform block/teture slot binding points

    static std::string precompile(const std::filesystem::path& filename);
    static GLuint compile(const std::string& source, GLenum type);
    static GLuint link(const std::initializer_list<GLuint>& shaders);
    void introspect();
};

template <typename T> void Shader::setUniformValue(const std::string& name, const T& value) {
    auto loc = getUniformLocation(name);
    if constexpr (std::is_same_v<T, bool> || std::is_same_v<T, int> || std::is_same_v<T, long> || std::is_same_v<T, long long>) {
        glProgramUniform1i(m_id, loc, value);
    } else if constexpr (std::is_same_v<T, unsigned int> || std::is_same_v<T, unsigned long> || std::is_same_v<T, unsigned long long>) {
        glProgramUniform1ui(m_id, loc, value);
    } else if constexpr (std::is_same_v<T, float>) {
        glProgramUniform1f(m_id, loc, value);
    } else if constexpr (std::is_same_v<T, double>) {
        glProgramUniform1d(m_id, loc, value);
    } else if constexpr (std::is_same_v<T, glm::vec2>) {
        glProgramUniform2fv(m_id, loc, 1, glm::value_ptr(value));
    } else if constexpr (std::is_same_v<T, glm::vec3>) {
        glProgramUniform3fv(m_id, loc, 1, glm::value_ptr(value));
    } else if constexpr (std::is_same_v<T, glm::vec4>) {
        glProgramUniform4fv(m_id, loc, 1, glm::value_ptr(value));
    } else if constexpr (std::is_same_v<T, glm::ivec2>) {
        glProgramUniform2iv(m_id, loc, 1, glm::value_ptr(value));
    } else if constexpr (std::is_same_v<T, glm::ivec3>) {
        glProgramUniform3iv(m_id, loc, 1, glm::value_ptr(value));
    } else if constexpr (std::is_same_v<T, glm::ivec4>) {
        glProgramUniform4iv(m_id, loc, 1, glm::value_ptr(value));
    } else if constexpr (std::is_same_v<T, glm::mat3>) {
        glProgramUniformMatrix3fv(m_id, loc, 1, GL_FALSE, glm::value_ptr(value));
    } else if constexpr (std::is_same_v<T, glm::mat4>) {
        glProgramUniformMatrix4fv(m_id, loc, 1, GL_FALSE, glm::value_ptr(value));
    } else {
        throw std::runtime_error("Shader::setUniformValue: uniform valuable set failed for " + name + " of type " + typeid(T).name() + ".");
    }
}

} // namespace tinyglrenderer

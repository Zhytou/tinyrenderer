#pragma once

#include <unordered_map>
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "scene.hpp"

namespace tinyrenderer
{
class Renderer
{
public:
	Renderer() : m_shadowMapWidth(1024), m_shadowMapHeight(1024) {}
	void setup();
	void render(const Scene& scene);

private:
	static GLuint compileShader(const std::string& filename, GLenum type);
	static GLuint linkProgram(std::initializer_list<GLuint> shaders);

	std::unordered_map<std::string, GLuint> m_programs;
	GLuint m_fbo;
	GLuint m_shadowMap;
	const int m_shadowMapWidth, m_shadowMapHeight;
};

} // namespace tinyrenderer
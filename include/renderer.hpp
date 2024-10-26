#pragma once

#include <unordered_map>
#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>


#include "scene.hpp"
#include "utils.hpp"

namespace tinyrenderer
{
class Renderer
{
public:
	void setup();
	void render(const Scene& scene);

private:
	static GLuint compileShader(const std::string& filename, GLenum type);
	static GLuint linkProgram(std::initializer_list<GLuint> shaders);

	std::unordered_map<std::string, GLuint> m_programs;
};

} // namespace tinyrenderer
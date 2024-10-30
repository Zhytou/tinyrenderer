#include <stdexcept>
#include <iostream>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "renderer.hpp"

namespace tinyrenderer
{

void Renderer::setup()
{
	// set global OpenGL state
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);

	// compile and link shaders
	m_programs["pbr"] = linkProgram({
		compileShader("../shader/pbr.vs", GL_VERTEX_SHADER),
		compileShader("../shader/pbr.fs", GL_FRAGMENT_SHADER)
	});
	
	glFinish();
}

void Renderer::render(const Scene& scene)
{
	// calculate view and projection matrices
	const glm::mat4 viewMatrix = glm::lookAt(scene.camera.eye, scene.camera.target, scene.camera.up);
	const glm::mat4 projectMatrix = glm::perspective(glm::radians(scene.camera.fov), scene.camera.aspect, scene.camera.near, scene.camera.far);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);        
	glClearDepth(1.0);

	glUseProgram(m_programs["pbr"]);
	// set MVP matrix uniforms
	glUniformMatrix4fv(glGetUniformLocation(m_programs["pbr"], "uViewMatrix"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
	glUniformMatrix4fv(glGetUniformLocation(m_programs["pbr"], "uProjectMatrix"), 1, GL_FALSE, glm::value_ptr(projectMatrix));

	// set camera and light uniforms
	glUniform3fv(glGetUniformLocation(m_programs["pbr"], "uCameraPos"), 1, glm::value_ptr(scene.camera.eye));

	for (int i = 0; i < maxLightNum; i++) {
		char s1[20], s2[20];

		std::sprintf(s1, "uLights[%d].position", i);
		glUniform3fv(glGetUniformLocation(m_programs["pbr"], s1), 1, glm::value_ptr(scene.lights[i].position));
		std::sprintf(s2, "uLights[%d].radiance", i);
		glUniform3fv(glGetUniformLocation(m_programs["pbr"], s2), 1, glm::value_ptr(scene.lights[i].radiance));
	}
		
	// iterate over models
	for (const auto& model : scene.models) {
		// set pbr textures
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, model.albedo.id);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, model.normal.id);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, model.metallic.id);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, model.roughness.id);
		
		// bind vertex array and draw
		glBindVertexArray(model.mesh.vao);
		glDrawElements(GL_TRIANGLES, model.mesh.indices.size(), GL_UNSIGNED_INT, 0);
	}
}


GLuint Renderer::compileShader(const std::string& filename, GLenum type)
{
	const std::string src = readText(filename);
	if(src.empty()) {
		throw std::runtime_error("Empty shader source file: " + filename);
	}
	const GLchar* srcBufferPtr = src.c_str();

	std::printf("Compiling GLSL shader: %s\n", filename.c_str());

	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &srcBufferPtr, nullptr);
	glCompileShader(shader);

	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if(status != GL_TRUE) {
		GLsizei infoLogSize;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogSize);
		std::unique_ptr<GLchar[]> infoLog(new GLchar[infoLogSize]);
		glGetShaderInfoLog(shader, infoLogSize, nullptr, infoLog.get());
		throw std::runtime_error(std::string("Shader compilation failed: ") + filename + "\n" + infoLog.get());
	}
	return shader;
}
	
GLuint Renderer::linkProgram(std::initializer_list<GLuint> shaders)
{
	GLuint program = glCreateProgram();

	for(GLuint shader : shaders) {
		glAttachShader(program, shader);
	}
	glLinkProgram(program);
	for(GLuint shader : shaders) {
		glDetachShader(program, shader);
		glDeleteShader(shader);
	}

	GLint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if(status == GL_TRUE) {
		glValidateProgram(program);
		glGetProgramiv(program, GL_VALIDATE_STATUS, &status);
	}
	if(status != GL_TRUE) {
		GLsizei infoLogSize;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogSize);
		std::unique_ptr<GLchar[]> infoLog(new GLchar[infoLogSize]);
		glGetProgramInfoLog(program, infoLogSize, nullptr, infoLog.get());
		throw std::runtime_error(std::string("Program link failed\n") + infoLog.get());
	}
	return program;
}

} // namespace tinyrenderer

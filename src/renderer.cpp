#include <stdexcept>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "renderer.hpp"

namespace tinyrenderer
{

void Renderer::setup()
{
	// set global OpenGL state
	glEnable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glFrontFace(GL_CCW);

	// compile and link shaders
	m_programs["phong"] = linkProgram({
		compileShader("../shader/phong.vs", GL_VERTEX_SHADER),
		compileShader("../shader/phong.fs", GL_FRAGMENT_SHADER)
	});
	
	glFinish();
}

void Renderer::render(const Scene& scene)
{
	// calculate view and projection matrices
	const glm::mat4 viewMatrix = glm::lookAt(scene.camera.eye, scene.camera.target, scene.camera.up);
	const glm::mat4 projectionMatrix = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 1000.0f);

	glEnable(GL_DEPTH_TEST);
	glUseProgram(m_programs["phong"]);
	// set MVP matrix uniforms
	glUniformMatrix4fv(glGetUniformLocation(m_programs["phong"], "uViewMatrix"), 1, GL_FALSE, &viewMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(m_programs["phong"], "uProjectMatrix"), 1, GL_FALSE, &projectionMatrix[0][0]);
	// set camera and light uniforms
	glUniform3fv(glGetUniformLocation(m_programs["phong"], "uCameraPos"), 1, &scene.camera.eye[0]);

	// iterate over lights
	for (const auto& light : scene.lights) {
		// set light uniforms
		glUniform3fv(glGetUniformLocation(m_programs["phong"], "uLightPos"), 1, &light.position[0]);
		glUniform3fv(glGetUniformLocation(m_programs["phong"], "uLightRadiance"), 1, &light.radiance[0]);
	
		// iterate over models
		for (const auto& model : scene.models) {
			
			for (const auto& mesh : model.meshes) {
				// set matrial uniforms
				if (mesh.materialId == -1) {
					glUniform3fv(glGetUniformLocation(m_programs["phong"], "uKa"), 1, &glm::vec3(0.0f)[0]);
					glUniform3fv(glGetUniformLocation(m_programs["phong"], "uKd"), 1, &glm::vec3(0.0f)[0]);
					glUniform3fv(glGetUniformLocation(m_programs["phong"], "uKs"), 1, &glm::vec3(0.0f)[0]);
					glUniform1f(glGetUniformLocation(m_programs["phong"], "uShininess"), 0.0f);
				} else {
					const Material& material = model.materials[mesh.materialId];
					glUniform3fv(glGetUniformLocation(m_programs["phong"], "uKa"), 1, &material.ambient[0]);
					glUniform3fv(glGetUniformLocation(m_programs["phong"], "uKd"), 1, &material.diffuse[0]);
					glUniform3fv(glGetUniformLocation(m_programs["phong"], "uKs"), 1, &material.specular[0]);
					glUniform1f(glGetUniformLocation(m_programs["phong"], "uShininess"), material.shininess);
				}

				// bind vertex array and draw
				glBindVertexArray(mesh.vao);
				glDrawElements(GL_TRIANGLES, sizeof(Face) * mesh.faces.size(), GL_UNSIGNED_INT, 0);
			}
		}
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

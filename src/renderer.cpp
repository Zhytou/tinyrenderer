#include <stdexcept>
#include <iostream>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "renderer.hpp"
#include "utils.hpp"

namespace tinyrenderer
{

void Renderer::setup()
{
	// set global OpenGL state
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glDepthFunc(GL_LESS);
	glClearDepth(1.0f);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	// compile and link shaders
	m_programs["shadow"] = linkProgram({
		compileShader("../shader/shadow.vs", GL_VERTEX_SHADER),
		compileShader("../shader/shadow.fs", GL_FRAGMENT_SHADER)
	});
	m_programs["pbr"] = linkProgram({
		compileShader("../shader/pbr.vs", GL_VERTEX_SHADER),
		compileShader("../shader/pbr.fs", GL_FRAGMENT_SHADER)
	});

	// create framebuffer object for shadow mapping
	glGenFramebuffers(1, &m_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

	// create shadow map texture
	glGenTextures(1, &m_shadowMap);
	glBindTexture(GL_TEXTURE_2D, m_shadowMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_shadowMapWidth, m_shadowMapHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	
	// set shadow map border color
	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	// attach shadow map texture to framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_shadowMap, 0);
	
	// set framebuffer to draw only depth
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::render(const Scene& scene)
{
	// calculate view and projection matrices
	const glm::mat4 viewMatrix = glm::lookAt(scene.camera.eye, scene.camera.target, scene.camera.up);
	const glm::mat4 projectMatrix = glm::perspective(glm::radians(scene.camera.fov), scene.camera.aspect, scene.camera.near, scene.camera.far);

	// calculate light space matrix
	const glm::mat4 lightViewMatrix = glm::lookAt(scene.dlight.direction, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	const glm::mat4 lightProjectMatrix = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 20.0f);
	const glm::mat4 lightSpaceMatrix = lightProjectMatrix * lightViewMatrix;
	
	// render shadow map of directional light
	{
		glUseProgram(m_programs["shadow"]);
		// bind framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
		// reset viewport
		glViewport(0, 0, m_shadowMapWidth, m_shadowMapHeight);
		// clear depth buffer
		glClear(GL_DEPTH_BUFFER_BIT);
		
		// set light MVP uniforms
		glUniformMatrix4fv(glGetUniformLocation(m_programs["shadow"], "uLightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

		for (const auto& model : scene.models) {
			// bind vertex array and draw
			glBindVertexArray(model.mesh.vao);
			glDrawElements(GL_TRIANGLES, model.mesh.indices.size(), GL_UNSIGNED_INT, 0);
		}
	}
	
	// render scene with PBR shading
	{
		glUseProgram(m_programs["pbr"]);
		// reset framebuffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// set MVP matrix uniforms
		glUniformMatrix4fv(glGetUniformLocation(m_programs["pbr"], "uViewMatrix"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
		glUniformMatrix4fv(glGetUniformLocation(m_programs["pbr"], "uProjectMatrix"), 1, GL_FALSE, glm::value_ptr(projectMatrix));
		glUniformMatrix4fv(glGetUniformLocation(m_programs["pbr"], "uLightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
		
		// set camera uniforms
		glUniform3fv(glGetUniformLocation(m_programs["pbr"], "uCameraPos"), 1, glm::value_ptr(scene.camera.eye));
		// set pointlight uniforms
		glUniform1i(glGetUniformLocation(m_programs["pbr"], "uNumPointLights"), scene.plights.size());
		for (int i = 0; i < scene.plights.size(); i++) {
			if (scene.plights[i].color == glm::vec3(0.0)) {
				continue;
			}
			char s1[40], s2[40];
			std::sprintf(s1, "uPointLights[%d].position", i);
			glUniform3fv(glGetUniformLocation(m_programs["pbr"], s1), 1, glm::value_ptr(scene.plights[i].position));
			std::sprintf(s2, "uPointLights[%d].color", i);
			glUniform3fv(glGetUniformLocation(m_programs["pbr"], s2), 1, glm::value_ptr(scene.plights[i].color));
		}
		// set directional light
		{
			glUniform3fv(glGetUniformLocation(m_programs["pbr"], "uDirectionalLight.direction"), 1, glm::value_ptr(scene.dlight.direction));
			glUniform3fv(glGetUniformLocation(m_programs["pbr"], "uDirectionalLight.color"), 1, glm::value_ptr(scene.dlight.color));
		}
		// iterate over models
		for (const auto& model : scene.models) {
			// set pbr textures
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, model.textures.at("albedo").id);
			if (model.textures.count("normal")) {
				glUniform1i(glGetUniformLocation(m_programs["pbr"], "uNormalMapped"), 1);
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, model.textures.at("normal").id);
			} else {
				glUniform1i(glGetUniformLocation(m_programs["pbr"], "uNormalMapped"), 0);
			}
			if (model.textures.count("metallic")) {
				glUniform1i(glGetUniformLocation(m_programs["pbr"], "uNormalMapped"), 1);
				glActiveTexture(GL_TEXTURE2);
				glBindTexture(GL_TEXTURE_2D, model.textures.at("metallic").id);
			} else {
				glUniform1i(glGetUniformLocation(m_programs["pbr"], "uMetllicMapped"), 0);
			}
			if (model.textures.count("roughness")) {
				glUniform1i(glGetUniformLocation(m_programs["pbr"], "uNormalMapped"), 1);
				glActiveTexture(GL_TEXTURE3);
				glBindTexture(GL_TEXTURE_2D, model.textures.at("roughness").id);
			} else {
				glUniform1i(glGetUniformLocation(m_programs["pbr"], "uRoughnessMapped"), 0);
			}
			if (model.textures.count("ao")) {
				glUniform1i(glGetUniformLocation(m_programs["pbr"], "uAOMapped"), 1);
				glActiveTexture(GL_TEXTURE4);
				glBindTexture(GL_TEXTURE_2D, model.textures.at("ao").id);
			} else {
				glUniform1i(glGetUniformLocation(m_programs["pbr"], "uAOMapped"), 0);
			}
			
			// bind vertex array and draw
			glBindVertexArray(model.mesh.vao);
			glDrawElements(GL_TRIANGLES, model.mesh.indices.size(), GL_UNSIGNED_INT, 0);
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

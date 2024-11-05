#include <stdexcept>
#include <filesystem>
#include <iostream>
#include <map>

#include <rapidjson/document.h>
#include <glm/gtc/matrix_transform.hpp>

#include "application.hpp"

namespace tinyrenderer
{
Application::Application(int width, int height, const std::string& title) : m_width(width), m_height(height)
{
	// glfw initialization
	if(!glfwInit()) {
		throw std::runtime_error("Failed to initialize GLFW library");
	}
	// set opengl version
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

	// set z-buffer bits, otherwise glenbale(GL_DEPTH_TEST) will not work
	glfwWindowHint(GLFW_DEPTH_BITS, 24);

	// glfw window creation
	m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
	if(!m_window) {
		throw std::runtime_error("Failed to create OpenGL context");
	}

	// glfw context settings(make sure it is all set before glad initialization)
	glfwMakeContextCurrent(m_window);
	glfwSwapInterval(-1);
	
	// glfw callbacks
	// set window pointer for later use in callback function
	glfwSetWindowUserPointer(m_window, this);
	// set callback function
	glfwSetWindowSizeCallback(m_window, [](GLFWwindow* window, int w, int h) {
		glViewport(0, 0, w, h);
	});
	// glfwSetCursorPosCallback(m_window, Application::mousePositionCallback);
	// glfwSetMouseButtonCallback(m_window, Application::mouseButtonCallback);
	// glfwSetScrollCallback(m_window, Application::mouseScrollCallback);

	// glad initialization(load all OpenGL function pointers)
	if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		throw std::runtime_error("Failed to initialize OpenGL extensions loader");
	}

	// compile and link shader program
	m_renderer.setup();

	std::printf("OpenGL Renderer [%s]\n", glGetString(GL_RENDERER));
}

Application::~Application()
{
	if(m_window) {
		glfwDestroyWindow(m_window);
	}
	glfwTerminate();
}

void Application::load(const std::string& configName, bool useDefaultCamera)
{
	std::string configJson = readText(configName);
	
	rapidjson::Document doc;
	if (doc.Parse(configJson.c_str()).HasParseError()) {
		throw std::runtime_error("Error parsing JSON");
	}
	
	if (doc.HasMember("speed")) {
		m_scene.speed = doc["speed"].GetFloat();
	} else {
		m_scene.speed = 0.1f;
	}

	auto lightsDoc = doc["lights"].GetObject();
	m_scene.plights.resize(lightsDoc["pointlight"].Size());
	for (int i = 0; i < m_scene.plights.size(); i++) {
		m_scene.plights[i].position = {
			lightsDoc["pointlight"][i]["position"][0].GetFloat(),
			lightsDoc["pointlight"][i]["position"][1].GetFloat(),
			lightsDoc["pointlight"][i]["position"][2].GetFloat()
		};
		m_scene.plights[i].color = {
			lightsDoc["pointlight"][i]["color"][0].GetFloat(),
			lightsDoc["pointlight"][i]["color"][1].GetFloat(),
			lightsDoc["pointlight"][i]["color"][2].GetFloat()
		};
	}
	{
		m_scene.dlight.direction = {
			lightsDoc["directionallight"]["direction"][0].GetFloat(),
			lightsDoc["directionallight"]["direction"][1].GetFloat(),
			lightsDoc["directionallight"]["direction"][2].GetFloat(),
		};
		m_scene.dlight.color = {
			lightsDoc["directionallight"]["color"][0].GetFloat(),
			lightsDoc["directionallight"]["color"][1].GetFloat(),
			lightsDoc["directionallight"]["color"][2].GetFloat(),
		};
	}

	AABB sceneAABB;
	auto modelsDoc = doc["models"].GetArray();
	for (int i = 0; i < modelsDoc.Capacity(); i++) {
		auto modelDoc = modelsDoc[i].GetObject();
		std::map<std::string, std::string> modelConfig;
		for (auto itr = modelDoc.begin(); itr != modelDoc.end(); itr++) {
			modelConfig[itr->name.GetString()] = itr->value.GetString();
		}
		Model model(modelConfig);
		sceneAABB.maxPos.x = std::max(sceneAABB.maxPos.x, model.aabb.maxPos.x);
		sceneAABB.maxPos.y = std::max(sceneAABB.maxPos.y, model.aabb.maxPos.y);
		sceneAABB.maxPos.z = std::max(sceneAABB.maxPos.z, model.aabb.maxPos.z);
		sceneAABB.minPos.x = std::min(sceneAABB.minPos.x, model.aabb.minPos.x);
		sceneAABB.minPos.y = std::min(sceneAABB.minPos.y, model.aabb.minPos.y);
		sceneAABB.minPos.z = std::min(sceneAABB.minPos.z, model.aabb.minPos.z);
		m_scene.models.emplace_back(model);
	}

	auto cameraDoc = doc["camera"].GetObject();
	m_scene.camera.eye = {
		cameraDoc["eye"][0].GetFloat(), 
		cameraDoc["eye"][1].GetFloat(), 
		cameraDoc["eye"][2].GetFloat()
	};
    m_scene.camera.target = {
		cameraDoc["target"][0].GetFloat(), 
		cameraDoc["target"][1].GetFloat(), 
		cameraDoc["target"][2].GetFloat()
	};
    m_scene.camera.up = {
		cameraDoc["up"][0].GetFloat(),
		cameraDoc["up"][1].GetFloat(),
		cameraDoc["up"][2].GetFloat()
	};
    m_scene.camera.fov = cameraDoc["fov"].GetFloat();
    m_scene.camera.aspect = m_width / m_height;
    m_scene.camera.near = cameraDoc["near"].GetFloat();
    m_scene.camera.far = cameraDoc["far"].GetFloat();

	if (useDefaultCamera) {
		glm::vec3 target = 0.5f * (sceneAABB.minPos + sceneAABB.maxPos);
		glm::vec3 eye = target + 2.0f * (sceneAABB.maxPos - sceneAABB.minPos);
		glm::vec3 direction = glm::normalize(sceneAABB.maxPos - sceneAABB.minPos);
		glm::vec3 up = {1.0f, 0.0f, 0.0f};
		if (std::abs(glm::dot(direction, up)) > 0.01f) {
			up = glm::cross(direction, up);
		}
		m_scene.camera.target = target;
		m_scene.camera.eye = eye;
		m_scene.camera.up = up;
	}

	std::printf("Camera eye(%f, %f, %f) target(%f, %f, %f)\n", m_scene.camera.eye.x, m_scene.camera.eye.y, m_scene.camera.eye.z, m_scene.camera.target.x, m_scene.camera.target.y, m_scene.camera.target.z);
}

void Application::run(AppMode mode)
{
	while(!glfwWindowShouldClose(m_window)) {
		// update scene
		switch (mode)
		{
		case AppMode::RotatingCamera:
		{
			glm::mat4 m(1.0f);
			m = glm::translate(m, -m_scene.camera.target);
			m = glm::rotate(m, glm::radians(m_scene.speed), glm::vec3(0.0f, 1.0f, 0.0f));
			m = glm::translate(m, m_scene.camera.target);
			m_scene.camera.eye = glm::vec3(m * glm::vec4(m_scene.camera.eye, 1.0f));
			break;
		}
		case AppMode::RotatingLight:
		{
			glm::mat4 m(1.0f);
			m = glm::rotate(m, glm::radians(m_scene.speed), glm::vec3(0.0f, 1.0f, 0.0f));
			m_scene.dlight.direction = glm::vec3(m * glm::vec4(m_scene.dlight.direction, 1.0f));
			printf("%f, %f, %f\n", m_scene.dlight.direction.x, m_scene.dlight.direction.y, m_scene.dlight.direction.z);
			break;
		}
		default:
			break;
		}
		
		// render scene
		m_renderer.render(m_scene);
		glfwSwapBuffers(m_window);
		glfwPollEvents();
	}
}

// void Application::mousePositionCallback(GLFWwindow* window, double xpos, double ypos)
// {
// 	Application* self = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
// 	if(self->m_mode != InputMode::None) {
// 		const double dx = xpos - self->m_prevCursorX;
// 		const double dy = ypos - self->m_prevCursorY;

// 		switch(self->m_mode) {
// 		case InputMode::RotatingScene:
// 			self->m_sceneSettings.yaw   += OrbitSpeed * float(dx);
// 			self->m_sceneSettings.pitch += OrbitSpeed * float(dy);
// 			break;
// 		case InputMode::RotatingView:
// 			self->m_viewSettings.yaw   += OrbitSpeed * float(dx);
// 			self->m_viewSettings.pitch += OrbitSpeed * float(dy);
// 			break;
// 		}

// 		self->m_prevCursorX = xpos;
// 		self->m_prevCursorY = ypos;
// 	}
// }
	
// void Application::mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
// {
// 	Application* self = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
// 	// self->m_camera += ZoomSpeed * float(-yoffset);
// }
	
} // namespace tinyrender

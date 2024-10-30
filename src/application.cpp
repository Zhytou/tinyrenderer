#include <stdexcept>
#include <filesystem>
#include <iostream>
#include <map>

#include <rapidjson/document.h>

#include "application.hpp"

namespace tinyrenderer
{
Application::Application(int width, int height, const std::string& title) : m_width(width), m_height(height)
{
	// glfw initialization
	if(!glfwInit()) {
		throw std::runtime_error("Failed to initialize GLFW library");
	}
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

	glfwWindowHint(GLFW_DEPTH_BITS, 0);
	glfwWindowHint(GLFW_STENCIL_BITS, 0);
	glfwWindowHint(GLFW_SAMPLES, 0);

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

void Application::load(const std::string& configName)
{
	std::string configJson = readText(configName);
	
	rapidjson::Document doc;
	if (doc.Parse(configJson.c_str()).HasParseError()) {
		throw std::runtime_error("Error parsing JSON");
	}
	
	auto camera = doc["camera"].GetObject();
	m_scene.camera.eye = {
		camera["eye"][0].GetFloat(), 
		camera["eye"][1].GetFloat(), 
		camera["eye"][2].GetFloat()
	};
    m_scene.camera.target = {
		camera["target"][0].GetFloat(), 
		camera["target"][1].GetFloat(), 
		camera["target"][2].GetFloat()
	};
    m_scene.camera.up = {
		camera["up"][0].GetFloat(),
		camera["up"][1].GetFloat(),
		camera["up"][2].GetFloat()
	};
    m_scene.camera.fov = camera["fov"].GetFloat();
    m_scene.camera.aspect = m_width / m_height;
    m_scene.camera.near = camera["near"].GetFloat();
    m_scene.camera.far = camera["far"].GetFloat();

	auto lights = doc["lights"].GetArray();
	for (int i = 0; i < std::min(maxLightNum, lights.Capacity()); i++) {
		m_scene.lights[i].position = {
			lights[i]["position"][0].GetFloat(),
			lights[i]["position"][1].GetFloat(),
			lights[i]["position"][2].GetFloat()
		};
		m_scene.lights[i].radiance = {
			lights[i]["radiance"][0].GetFloat(),
			lights[i]["radiance"][1].GetFloat(),
			lights[i]["radiance"][2].GetFloat()
		};
	}

	auto models = doc["models"].GetArray();
	for (int i = 0; i < models.Capacity(); i++) {
		auto model = models[i].GetObject();
		std::map<std::string, std::string> modelConfig;
		for (auto itr = model.begin(); itr != model.end(); itr++) {
			modelConfig[itr->name.GetString()] = itr->value.GetString();
		}
		m_scene.models.emplace_back(modelConfig);
	}
	
}

void Application::run()
{
	while(!glfwWindowShouldClose(m_window)) {
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
	
// void Application::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
// {
// 	Application* self = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));

// 	const InputMode oldMode = self->m_mode;
// 	if(action == GLFW_PRESS && self->m_mode == InputMode::None) {
// 		switch(button) {
// 		case GLFW_MOUSE_BUTTON_1:
// 			self->m_mode = InputMode::RotatingView;
// 			break;
// 		case GLFW_MOUSE_BUTTON_2:
// 			self->m_mode = InputMode::RotatingScene;
// 			break;
// 		}
// 	}
// 	if(action == GLFW_RELEASE && (button == GLFW_MOUSE_BUTTON_1 || button == GLFW_MOUSE_BUTTON_2)) {
// 		self->m_mode = InputMode::None;
// 	}

// 	if(oldMode != self->m_mode) {
// 		if(self->m_mode == InputMode::None) {
// 			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
// 		}
// 		else {
// 			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
// 			glfwGetCursorPos(window, &self->m_prevCursorX, &self->m_prevCursorY);
// 		}
// 	}
// }
	
// void Application::mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
// {
// 	Application* self = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
// 	// self->m_camera += ZoomSpeed * float(-yoffset);
// }
	
} // namespace tinyrender

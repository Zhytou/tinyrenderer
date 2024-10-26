#include <stdexcept>
#include <filesystem>
#include <iostream>

#include "application.hpp"

namespace tinyrenderer
{
Application::Application()
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
	m_window = glfwCreateWindow(700, 500, "Physically Based Rendering (OpenGL 4.5)", nullptr, nullptr);
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
	// glfwSetCursorPosCallback(m_window, Application::mousePositionCallback);
	// glfwSetMouseButtonCallback(m_window, Application::mouseButtonCallback);
	// glfwSetScrollCallback(m_window, Application::mouseScrollCallback);

	// glad initialization(load all OpenGL function pointers)
	if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		throw std::runtime_error("Failed to initialize OpenGL extensions loader");
	}

	std::printf("OpenGL 4.5 Renderer [%s]\n", glGetString(GL_RENDERER));
}

Application::~Application()
{
	if(m_window) {
		glfwDestroyWindow(m_window);
	}
	glfwTerminate();
}

void Application::load(const std::string& baseDir, const std::string& modelName, const std::string& configName)
{
	m_scene.models = {Model(baseDir, modelName)};
	
	m_scene.camera.eye = {278, 273, -800};
	m_scene.camera.target = {278, 273, -799};
	m_scene.camera.up = {0, 1, 0};

	m_scene.lights = {
		{{343.0, 548.7, 227.0 }, {1, 1, 1}},
	};
}

void Application::run()
{
	glfwWindowHint(GLFW_RESIZABLE, 0);

	m_renderer.setup();
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

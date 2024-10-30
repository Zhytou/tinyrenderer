#pragma once

#include <memory>

#include "renderer.hpp"
#include "scene.hpp"

namespace tinyrenderer
{

class Application
{
public:
	// create application window and initialize OpenGL context
	Application(int width, int height, const std::string& title);
	
	// destroy application window and release resources
	~Application();
	
	// load model, light and camera
	void load(const std::string& configName);

	// run application main loop
	void run();

private:
	// static void mousePositionCallback(GLFWwindow* window, double xpos, double ypos);
	// static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	// static void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

	GLFWwindow* m_window;
	Renderer m_renderer;
	Scene m_scene;
	int m_width, m_height;
};
	
} // namespace tinyrenderer

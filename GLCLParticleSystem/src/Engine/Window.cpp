#include "glclpch.h"
#include "Engine/Window.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Engine
{

	Window::Window(const std::string& name, uint32_t width, uint32_t height, bool maximize)
		:m_Name(name), m_Width(width), m_Height(height)
	{
		Initialize(name, width, height, maximize);
	}

	Window::~Window()
	{
		glfwDestroyWindow(m_WindowHandle);
		glfwTerminate();
	}

	void Window::Update()
	{
		glfwPollEvents();
		glfwSwapBuffers(m_WindowHandle);
	}

	void Window::Initialize(const std::string& name, uint32_t width, uint32_t height, bool maximize)
	{
		if (!glfwInit())
		{
			LOG_ERROR("Failure to initialize GLFW!");
			return;
		}
		m_WindowHandle = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);

		glfwMakeContextCurrent(m_WindowHandle);

		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			LOG_ERROR("Failure to intialize glad OpenGL context!");
			return;
		}

		if(maximize)
			glfwMaximizeWindow(m_WindowHandle);
	}

	bool Window::IsOpen() const
	{
		return m_WindowHandle != nullptr && !glfwWindowShouldClose(m_WindowHandle);
	}
}
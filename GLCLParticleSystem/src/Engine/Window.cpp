#include "glclpch.h"
#include "Engine/Window.h"
#include "Engine/Event/KeyEvent.h"
#include "Engine/Event/MouseEvent.h"
#include "Engine/Event/WindowEvent.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Engine
{
	Window* Window::s_Instance = nullptr;

	void Window::Create(const std::string& name, uint32_t width, uint32_t height, bool maximize)
	{
		if (s_Instance != nullptr) return;

		s_Instance = new Window(name, width, height, maximize);
	}

	void Window::Shutdown()
	{
		if (s_Instance == nullptr) return;

		delete s_Instance;
	}

	bool Window::IsOpen()
	{
		return s_Instance->m_WindowHandle != nullptr && !glfwWindowShouldClose(s_Instance->m_WindowHandle);
	}

	void Window::Update()
	{
		glfwPollEvents();
		glfwSwapBuffers(s_Instance->m_WindowHandle);
	}

	void Window::SetEventCallbackFunction(const EventCallbackFunction& callback)
	{
		if (s_Instance == nullptr) return;

		s_Instance->m_WindowData.Callback = callback;
	}

	Window::Window(const std::string& name, uint32_t width, uint32_t height, bool maximize)
	{
		Initialize(name, width, height, maximize);
	}

	Window::~Window()
	{
		glfwDestroyWindow(m_WindowHandle);
		glfwTerminate();
	}

	void Window::Initialize(const std::string& name, uint32_t width, uint32_t height, bool maximize)
	{
		m_WindowData = WindowData(name, width, height);
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

		glfwSetWindowUserPointer(m_WindowHandle, static_cast<void*>(&m_WindowData));

		glfwSetWindowCloseCallback(m_WindowHandle, [](GLFWwindow* window)
			{
				WindowData& data = *(static_cast<WindowData*>(glfwGetWindowUserPointer(window)));

				WindowClosedEvent windowClosedEvent;
				data.Callback(windowClosedEvent);
			});

		glfwSetWindowSizeCallback(m_WindowHandle, [](GLFWwindow* window, int width, int height)
			{
				WindowData& data = *(static_cast<WindowData*>(glfwGetWindowUserPointer(window)));
				data.Width = width;
				data.Height = height;

				WindowResizedEvent windowResizedEvent(width, height);
				data.Callback(windowResizedEvent);
			});

		glfwSetWindowPosCallback(m_WindowHandle, [](GLFWwindow* window, int xPosition, int yPosition)
			{
				WindowData& data = *(static_cast<WindowData*>(glfwGetWindowUserPointer(window)));

				WindowMovedEvent windowMovedEvent(xPosition, yPosition);
				data.Callback(windowMovedEvent);
			});

		glfwSetKeyCallback(m_WindowHandle, [](GLFWwindow* window, int keyCode, int scanCode, int action, int mods)
			{
				WindowData& data = *(static_cast<WindowData*>(glfwGetWindowUserPointer(window)));

				switch (action)
				{
				case GLFW_PRESS:
				{
					KeyPressedEvent keyPressedEvent(keyCode, 0);
					data.Callback(keyPressedEvent);
					break;
				}
				case GLFW_REPEAT:
				{
					KeyPressedEvent keyPressedEvent(keyCode, 1);
					data.Callback(keyPressedEvent);
					break;
				}
				case GLFW_RELEASE:
				{
					KeyReleasedEvent keyReleasedEvent(keyCode);
					data.Callback(keyReleasedEvent);
					break;
				}
				}
			});

		glfwSetMouseButtonCallback(m_WindowHandle, [](GLFWwindow* window, int button, int action, int mods)
			{
				WindowData& data = *(static_cast<WindowData*>(glfwGetWindowUserPointer(window)));

				switch (action)
				{
				case GLFW_PRESS:
				{
					MouseButtonPressedEvent mouseButtonPressedEvent(button);
					data.Callback(mouseButtonPressedEvent);
					break;
				}
				case GLFW_RELEASE:
				{
					MouseButtonReleasedEvent mouseButtonReleasedEvent(button);
					data.Callback(mouseButtonReleasedEvent);
					break;
				}
				}
			});

		glfwSetScrollCallback(m_WindowHandle, [](GLFWwindow* window, double xOffset, double yOffset)
			{
				WindowData& data = *(static_cast<WindowData*>(glfwGetWindowUserPointer(window)));

				MouseScrolledEvent mouseScrolledEvent(static_cast<float>(xOffset), static_cast<float>(yOffset));
				data.Callback(mouseScrolledEvent);
			});


		glfwSetCursorPosCallback(m_WindowHandle, [](GLFWwindow* window, double xPosition, double yPosition)
			{
				WindowData& data = *(static_cast<WindowData*>(glfwGetWindowUserPointer(window)));

				MouseMovedEvent mouseMovedEvent(static_cast<float>(xPosition), static_cast<float>(yPosition));
				data.Callback(mouseMovedEvent);
			});
	}
}
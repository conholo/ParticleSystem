#pragma once

#include "Engine/Event/Event.h"


struct GLFWwindow;

namespace Engine
{
	class Window
	{
	public:
		using EventCallbackFunction = std::function<void(Event&)>;

		static void Update();
		static void Create(const std::string& name, uint32_t width, uint32_t height, bool maximize = true);
		static void SetEventCallbackFunction(const EventCallbackFunction& callback);
		static void Shutdown();


		static bool IsOpen();
		static GLFWwindow* GetWindow() { return s_Instance->m_WindowHandle; }
		static uint32_t GetWidth() { return s_Instance->m_WindowData.Width; }
		static uint32_t GetHeight() { return s_Instance->m_WindowData.Height; }

	private:
		Window(const std::string& name, uint32_t width, uint32_t height, bool maximize = true);
		~Window();
		void Initialize(const std::string& name, uint32_t width, uint32_t height, bool maximize);

	private:

		struct WindowData
		{
			std::string Name;
			uint32_t Width, Height;
			EventCallbackFunction Callback;


			WindowData(const std::string& name = "Window", uint32_t width = 1920, uint32_t height = 1080)
				:Name(name), Width(width), Height(height)
			{
			}
		};

		WindowData m_WindowData;

		static Window* s_Instance;
		GLFWwindow* m_WindowHandle;
	};
}

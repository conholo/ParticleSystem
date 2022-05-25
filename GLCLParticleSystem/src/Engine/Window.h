#pragma once


struct GLFWwindow;

namespace Engine
{
	class Window
	{
	public:
		Window(const std::string& name, uint32_t width, uint32_t height, bool maximize = true);
		~Window();

		void Update();
		bool IsOpen() const;

	private:
		void Initialize(const std::string& name, uint32_t width, uint32_t height, bool maximize);


	private:
		GLFWwindow* m_WindowHandle;
		uint32_t m_Width;
		uint32_t m_Height;
		std::string m_Name;
	};
}

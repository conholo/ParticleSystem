#pragma once

#include "Engine/Event/Event.h"
#include "Engine/Event/WindowEvent.h"
#include "Engine/Event/KeyEvent.h"
#include "Particle/ParticleSystem.h"
#include "Engine/Renderer/Camera.h"

namespace Engine
{
	class Application
	{
	public:
		static void Run();
		void OnEvent(Event& event);

		static Application& GetApplication() { return *s_Instance; }
		const std::string& GetName() const { return m_Name; }

		static void Create(const std::string& name = "Application");
		static void Shutdown();

	private:
		Application(const std::string& name);
		~Application();
		
	private:
		bool OnWindowClose(WindowClosedEvent& windowCloseEvent);
		bool OnWindowResize(WindowResizedEvent& windowResizeEvent);
		bool OnKeyPressed(KeyPressedEvent& keyPressedEvent);

	private:
		static Application* s_Instance;

	private:
		Camera m_Camera;
		ParticleSystem* m_PS;
		bool m_IsRunning = true;
		std::string m_Name;
	};
}
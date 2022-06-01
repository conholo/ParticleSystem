#include "glclpch.h"

#include "Engine/Window.h"
#include "Engine/Compute/OpenCLContext.h"
#include "Engine/Application.h"
#include "Engine/Time.h"
#include "Engine/Renderer/RenderCommand.h"
#include "Engine/Random.h"

#include <GLFW/glfw3.h>

namespace Engine
{
	Application* Application::s_Instance = nullptr;

	void Application::Create(const std::string& name)
	{
		if (s_Instance != nullptr) return;

		s_Instance = new Application(name);
	}

	void Application::Shutdown()
	{
		s_Instance->m_IsRunning = false;
		delete s_Instance;
	}

	Application::Application(const std::string& name)
		:m_Name(name)
	{
		Window::Create(name, 1920, 1080);
		Window::SetEventCallbackFunction(BIND_FN(OnEvent));
		Log::Initialize();
		Random::Initialize();
		RenderCommand::Initialize();
		RenderCommand::SetViewport(Window::GetWidth(), Window::GetHeight());
		OpenCLContext::Initialize();

		ParticleSystemProperties properties;
		m_PS = new Engine::ParticleSystem(properties, "resources/cl/particle_sim.cl", "resources/shaders/particle_shader.shader");

		m_Camera.SetPosition({ 0.0f, 0.0f, 5.0f });
		m_Camera.SetPerspective();
	}

	Application::~Application()
	{
		delete m_PS;
		Window::Shutdown();
	}

	void Application::Run()
	{
		while (s_Instance->m_IsRunning)
		{
			Time::Tick();
			s_Instance->m_Camera.Update(Time::DeltaTime());
			RenderCommand::Clear(true, true);
			RenderCommand::ClearColor({ 0.1f, 0.1f, 0.1f, 0.1f });

			s_Instance->m_PS->Tick(Time::DeltaTime());
			s_Instance->m_PS->Render(s_Instance->m_Camera);

			Window::Update();
		}
	}

	void Application::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);

		dispatcher.Dispatch<WindowClosedEvent>(BIND_FN(Application::OnWindowClose));
		dispatcher.Dispatch<WindowResizedEvent>(BIND_FN(Application::OnWindowResize));
		m_Camera.OnEvent(event);
	}

	bool Application::OnWindowClose(WindowClosedEvent& windowCloseEvent)
	{
		m_IsRunning = false;
		return true;
	}

	bool Application::OnWindowResize(WindowResizedEvent& windowResizeEvent)
	{
		RenderCommand::SetViewport(windowResizeEvent.GetWidth(), windowResizeEvent.GetHeight());
		return false;
	}
}

#include "glclpch.h"
#include "Engine/Application.h"


int main()
{
	Engine::Application::Create("Particle System");
	Engine::Application::Run();
	Engine::Application::Shutdown();
}
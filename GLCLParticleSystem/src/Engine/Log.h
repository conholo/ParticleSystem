#pragma once

#include "glm/gtx/string_cast.hpp"

#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#pragma warning(pop)

namespace Engine
{
	class Log
	{
	public:
		static void Initialize();

		static std::shared_ptr<spdlog::logger> GetEngineLogger() { return s_EngineLogger; }
		static std::shared_ptr<spdlog::logger> GetClientLogger() { return s_ClientLogger; }

	private:
		static std::shared_ptr<spdlog::logger> s_EngineLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};
}


#define LOG_TRACE(...)		::Engine::Log::GetEngineLogger()->trace(__VA_ARGS__)
#define LOG_INFO(...)		::Engine::Log::GetEngineLogger()->info(__VA_ARGS__)
#define LOG_WARN(...)		::Engine::Log::GetEngineLogger()->warn(__VA_ARGS__)
#define LOG_ERROR(...)		::Engine::Log::GetEngineLogger()->error(__VA_ARGS__)
#define LOG_CRITICAL(...)	::Engine::Log::GetEngineLogger()->critical(__VA_ARGS__)
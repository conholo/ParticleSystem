workspace "GLCLParticleSystem"
	architecture "x64"
	startproject "GLCLParticleSystem"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDirectories = {}
IncludeDirectories["GLFW"]	= "%{wks.location}/Dependencies/GLFW/include"
IncludeDirectories["glad"]	= "%{wks.location}/Dependencies/glad/include"


group "Dependencies"
	include "Dependencies/GLFW"
	include "Dependencies/glad"
group ""

project "GLCLParticleSystem"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	location "GLCLParticleSystem"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"GLFW_INCLUDE_NONE"
	}

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/vendor/glm/glm/**.hpp",
		"%{prj.name}/vendor/glm/glm/**.inl",
	}

	includedirs 
	{
		"%{prj.name}/src",
		"%{prj.name}/vendor/glm",
		"%{prj.name}/vendor/spdlog/include",
		"%{prj.name}/vendor/OpenCL/include",
		"%{IncludeDirectories.GLFW}",
		"%{IncludeDirectories.glad}",
	}

	libdirs
	{
		"%{wks.location}/Dependencies/OpenCL"
	}

	links
	{
		"GLFW",
		"glad",
		"OpenCL64.lib"
	}

	pchheader "glclpch.h"
	pchsource "%{prj.name}/src/glclpch.cpp"

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		defines "GLCL_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "GLCL_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "GLCL_DIST"
		runtime "Release"
		optimize "on"

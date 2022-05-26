#pragma once

#include <glm/glm.hpp>
#include "Particle/SimulationBounds.h"
#include "Engine/Renderer/VertexArray.h"
#include "Engine/Renderer/Shader.h"
#include "Engine/Compute/OpenCLProgram.h"
#include "Engine/Renderer/Camera.h"

namespace Engine
{
	struct ParticleSystemProperties
	{
		ParticleSystemProperties(
			size_t particleCount = 100000,
			const SimulationBounds& bounds = SimulationBounds(),
			const glm::vec3& minVelocity = glm::vec3(-1.0f),
			const glm::vec3& maxVelocity = glm::vec3(1.0f),
			bool randomColors = true,
			const glm::vec4& defaultPingColor = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f),
			const glm::vec4& defaultPongColor = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f))
			: 
			ParticleCount(particleCount), 
			Bounds(bounds), 
			MinVelocity(minVelocity), MaxVelocity(maxVelocity), 
			RandomColors(randomColors), 
			PingColor(defaultPingColor), PongColor(defaultPongColor)
		{
		}

		size_t VelocityDataByteSize;
		size_t PositionDataByteSize;
		size_t ColorDataByteSize;

		bool RandomColors;
		glm::vec4 PingColor;
		glm::vec4 PongColor;
		size_t ParticleCount;
		SimulationBounds Bounds;
		glm::vec3 MinVelocity;
		glm::vec3 MaxVelocity;
	};

	class ParticleSystem
	{
	public:
		ParticleSystem(const ParticleSystemProperties& properties, const std::string& clKernelFilePath, const std::string& shaderFilePath);
		~ParticleSystem();

		void Tick(float dt);
		void Render(const Camera& camera);
		void Reset();

	private:
		void Initialize(const std::string& clKernelFilePath, const std::string& shaderFilePath);

	private:
		Shader* m_ParticlePointShader;
		VertexArray* m_VAO;
		OpenCLProgram* m_ParticleProgram;
		OpenCLBuffer* m_CLVelocityBuffer;
		OpenCLBuffer* m_CLPositionBuffer;
		OpenCLBuffer* m_CLColorBuffer;
		OpenCLKernel* m_ParticleSimulationKernel;

		glm::ivec3 m_GlobalWorkSize;
		glm::ivec3 m_LocalWorkSize;
		const size_t c_ThreadsPerWorkGroup = 64;

		VertexBuffer* m_ParticlePositionVBO;
		VertexBuffer* m_ParticleColorVBO;
		std::vector<glm::vec4> m_Velocities;

		ParticleSystemProperties m_Properties;
	};
}

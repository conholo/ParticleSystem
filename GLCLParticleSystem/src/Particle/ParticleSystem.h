#pragma once

#include <glm/glm.hpp>
#include "Particle/SimulationBounds.h"
#include "Particle/SimulationWorld.h"
#include "Engine/Renderer/VertexArray.h"
#include "Engine/Renderer/Shader.h"
#include "Engine/Compute/OpenCLProgram.h"
#include "Engine/Renderer/Camera.h"

#include <OpenCL/cl.h>

namespace Engine
{
	struct cl_simulation_bounds
	{
		cl_float4 Center;
		cl_float4 MinExtent;
		cl_float4 MaxExtent;
	};

	struct ParticleSystemProperties
	{
		ParticleSystemProperties(
			size_t particleCount = 1024 * 1024 * 16,
			const glm::vec3& minVelocity = glm::vec3(-1.0f),
			const glm::vec3& maxVelocity = glm::vec3(1.0f))
			: 
			ParticleCount(particleCount), 
			MinVelocity(minVelocity), MaxVelocity(maxVelocity) 
		{
		}

		size_t VelocityDataByteSize;
		size_t PositionDataByteSize;
		size_t ColorDataByteSize;

		size_t ParticleCount;
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
		void ApplyPulse();
		void ToggleRenderSpheres() const { m_World->ToggleRenderSpheres(); }
		void Start() { m_Start = true; }

		const ParticleSystemProperties& GetProperties() const { return m_Properties; }
		double GetAverageFrameTime() const { return m_ParticleProgram->GetSumTime() / m_MaxFrameCount * 1000.0f; }
		bool IsFinished() const { return m_FrameCounter >= m_MaxFrameCount; }

	private:
		void UpdateBounds();
		void Initialize(const std::string& clKernelFilePath, const std::string& shaderFilePath);

	private:

		size_t m_FrameCounter = 0;
		size_t m_MaxFrameCount = 1000;
		bool m_Start = false;
		cl_float* m_TimePtr;
		cl_float4* m_SpheresPtr;
		cl_simulation_bounds* m_CLBoundsPtr;

		SimulationWorld* m_World;
		Shader* m_ParticlePointShader;
		VertexArray* m_VAO;
		OpenCLProgram* m_ParticleProgram;
		OpenCLBuffer* m_CLVelocityBuffer;
		OpenCLBuffer* m_CLPositionBuffer;
		OpenCLBuffer* m_CLColorBuffer;
		OpenCLBuffer* m_SimulationBoundsBuffer;
		OpenCLBuffer* m_SpheresBuffer;
		OpenCLBuffer* m_TimeBuffer;
		OpenCLKernel* m_ParticleSimulationKernel;

		OpenCLKernel* m_PulseKernel;


		float m_RotationSpeed = 1.0f;

		glm::ivec3 m_GlobalWorkSize;
		glm::ivec3 m_LocalWorkSize;
		const size_t c_ThreadsPerWorkGroup = 64;

		VertexBuffer* m_ParticlePositionVBO;
		VertexBuffer* m_ParticleColorVBO;
		std::vector<glm::vec4> m_Velocities;

		ParticleSystemProperties m_Properties;
	};
}

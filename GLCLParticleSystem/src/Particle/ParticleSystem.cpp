#include "glclpch.h"
#include "Particle/ParticleSystem.h"
#include "Engine/Renderer/RenderCommand.h"

#include "Engine/Random.h"
#include <glm/glm.hpp>

namespace Engine
{
	ParticleSystem::ParticleSystem(const ParticleSystemProperties& properties, const std::string& clKernelFilePath, const std::string& shaderFilePath)
		:m_Properties(properties)
	{
		size_t dataSize = properties.ParticleCount * sizeof(float) * 4;
		m_Properties.ColorDataByteSize = m_Properties.PositionDataByteSize = m_Properties.VelocityDataByteSize = dataSize;

		m_GlobalWorkSize = glm::ivec3(properties.ParticleCount, 1, 1);
		m_LocalWorkSize = glm::ivec3(c_ThreadsPerWorkGroup, 1, 1);

		Initialize(clKernelFilePath, shaderFilePath);
		Reset();
	}

	ParticleSystem::~ParticleSystem()
	{
		delete m_ParticleProgram;
		delete m_ParticleColorVBO;
		delete m_ParticlePositionVBO;
		delete m_VAO;
		delete m_ParticlePointShader;
	}

	void ParticleSystem::Initialize(const std::string& clKernelFilePath, const std::string& shaderFilePath)
	{
		m_ParticlePointShader = new Shader(shaderFilePath);
		m_VAO = new VertexArray;
		m_ParticlePositionVBO = new VertexBuffer(m_Properties.PositionDataByteSize);
		m_ParticlePositionVBO->SetLayout({ {"a_Position", ShaderDataType::Float4} });
		m_ParticleColorVBO = new VertexBuffer(m_Properties.ColorDataByteSize);
		m_ParticleColorVBO->SetLayout({ {"a_Color", ShaderDataType::Float4} });
		m_VAO->AddVertexArray(m_ParticlePositionVBO);
		m_VAO->AddVertexArray(m_ParticleColorVBO);
		m_Velocities.resize(m_Properties.ParticleCount);
	
		m_ParticleProgram =			new OpenCLProgram(clKernelFilePath);
		m_CLVelocityBuffer =		new OpenCLBuffer(m_ParticleProgram, "velocityBuffer", m_Properties.VelocityDataByteSize, CLBufferType::ReadWrite);
		m_CLPositionBuffer =		new OpenCLBuffer(m_ParticleProgram, "positionBuffer", m_Properties.PositionDataByteSize, CLBufferType::ReadWrite, m_ParticlePositionVBO);
		m_CLColorBuffer =			new OpenCLBuffer(m_ParticleProgram, "colorBuffer", m_Properties.ColorDataByteSize, CLBufferType::ReadWrite, m_ParticleColorVBO);
		m_SimulationBoundsBuffer =	new OpenCLBuffer(m_ParticleProgram, "boundsBuffer", sizeof(cl_simulation_bounds), CLBufferType::ReadOnly);

		glm::vec3 boundsCenter = m_Properties.Bounds.GetCenter();
		cl_float4 center = { boundsCenter.x, boundsCenter.y, boundsCenter.z, 1.0f };

		glm::vec3 maxExtent = m_Properties.Bounds.GetMaxExtents();
		cl_float4 max = { maxExtent.x, maxExtent.y, maxExtent.z, 1.0f };

		glm::vec3 minExtent = m_Properties.Bounds.GetMinExtents();
		cl_float4 min = { minExtent.x, minExtent.y, minExtent.z, 1.0f };

		m_CLBoundsPtr = new cl_simulation_bounds();
		m_CLBoundsPtr->Center = center;
		m_CLBoundsPtr->MaxExtent = max;
		m_CLBoundsPtr->MinExtent = min;

		m_ParticleSimulationKernel = new OpenCLKernel(m_ParticleProgram, "ParticleSimulation",
			{
				new KernelArg(m_CLPositionBuffer->GetBufferName(),			m_CLPositionBuffer->GetBufferID(),			OpenCLBuffer::NativeSize(),		KernelArgType::Global),
				new KernelArg(m_CLVelocityBuffer->GetBufferName(),			m_CLVelocityBuffer->GetBufferID(),			OpenCLBuffer::NativeSize(),		KernelArgType::Global),
				new KernelArg(m_CLColorBuffer->GetBufferName(),				m_CLColorBuffer->GetBufferID(),				OpenCLBuffer::NativeSize(),		KernelArgType::Global),
				new KernelArg(m_SimulationBoundsBuffer->GetBufferName(),	m_SimulationBoundsBuffer->GetBufferID(),	OpenCLBuffer::NativeSize(),		KernelArgType::Global),
			});

		m_ParticleProgram->AddBuffer(m_CLPositionBuffer);
		m_ParticleProgram->AddBuffer(m_CLVelocityBuffer);
		m_ParticleProgram->AddBuffer(m_CLColorBuffer);
		m_ParticleProgram->AddBuffer(m_SimulationBoundsBuffer);
		m_ParticleProgram->AddKernel(m_ParticleSimulationKernel);
		m_ParticleSimulationKernel->AttachArgs();

		m_ParticleProgram->WriteToDeviceBufferFromHostBuffer("boundsBuffer", sizeof(cl_simulation_bounds), m_CLBoundsPtr);
	}

	void ParticleSystem::Tick(float dt)
	{
		m_ParticleProgram->EnqueueAcquireGLObjects("positionBuffer");
		m_ParticleProgram->EnqueueAcquireGLObjects("colorBuffer");
		m_ParticleProgram->Execute("ParticleSimulation", m_GlobalWorkSize, m_LocalWorkSize, 0);
		m_ParticleProgram->Flush();
		m_ParticleProgram->EnqueueReleaseGLObjects("positionBuffer");
		m_ParticleProgram->EnqueueReleaseGLObjects("colorBuffer");
	}

	void ParticleSystem::Render(const Camera& camera)
	{
		m_VAO->EnableVertexAttributes();
		m_ParticlePointShader->Bind();
		m_ParticlePointShader->UploadUniformMat4("u_ViewProjectionMatrix", camera.GetViewProjection());
		RenderCommand::DrawPoints(m_Properties.ParticleCount);
	}

	void ParticleSystem::Reset()
	{
		const glm::vec3 minExtents = m_Properties.Bounds.GetMinExtents();
		const glm::vec3 maxExtents = m_Properties.Bounds.GetMaxExtents();

		float radius = abs(m_Properties.Bounds.GetMaxExtents().x - m_Properties.Bounds.GetMaxExtents().x) / 4.0f;

		glm::vec4* positions = (glm::vec4*)m_ParticlePositionVBO->MapBuffer(m_Properties.PositionDataByteSize, BufferHint::WriteOnly);
		for (int i = 0; i < m_Properties.ParticleCount; i++)
		{
			positions[i].x = Random::RandomRange(minExtents.x, maxExtents.x);
			positions[i].y = Random::RandomRange(minExtents.y, maxExtents.y);
			positions[i].z = Random::RandomRange(minExtents.z, maxExtents.z);
			positions[i].w = 1.0f;
		}
		m_ParticlePositionVBO->UnmapBuffer();

		glm::vec4* colors = (glm::vec4*)m_ParticleColorVBO->MapBuffer(m_Properties.ColorDataByteSize, BufferHint::WriteOnly);
		for (int i = 0; i < m_Properties.ParticleCount; i++)
		{
			if (m_Properties.RandomColors)
			{
				colors[i].r = Random::RandomRange(0.0f, 1.0f);
				colors[i].g = Random::RandomRange(0.0f, 1.0f);
				colors[i].b = Random::RandomRange(0.0f, 1.0f);
				colors[i].a = 1.0f;
			}
			else
			{
				colors[i] = m_Properties.PingColor;
			}
		}
		m_ParticleColorVBO->UnmapBuffer();

		for (int i = 0; i < m_Properties.ParticleCount; i++)
		{
			m_Velocities[i].x = Random::RandomRange(m_Properties.MinVelocity.x, m_Properties.MaxVelocity.x);
			m_Velocities[i].y = Random::RandomRange(m_Properties.MinVelocity.y, m_Properties.MaxVelocity.y);
			m_Velocities[i].z = Random::RandomRange(m_Properties.MinVelocity.z, m_Properties.MaxVelocity.z);
			m_Velocities[i].w = 0.0f;
		}

		m_ParticleProgram->WriteToDeviceBufferFromHostBuffer("velocityBuffer", m_Properties.VelocityDataByteSize, m_Velocities.data());
	}
}
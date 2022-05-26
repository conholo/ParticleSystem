#include "glclpch.h"
#include "Particle/ParticleSystem.h"
#include "Engine/Renderer/RenderCommand.h"

#include "Engine/Random.h"

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
	

		m_ParticleProgram = new OpenCLProgram(clKernelFilePath);
		m_CLVelocityBuffer = new OpenCLBuffer(m_ParticleProgram, "velocityBuffer", m_Properties.VelocityDataByteSize, CLBufferType::ReadWrite);
		m_CLPositionBuffer = new OpenCLBuffer(m_ParticleProgram, "positionBuffer", m_Properties.PositionDataByteSize, CLBufferType::ReadWrite, m_ParticlePositionVBO);
		m_CLColorBuffer = new OpenCLBuffer(m_ParticleProgram, "colorBuffer", m_Properties.ColorDataByteSize, CLBufferType::ReadWrite, m_ParticleColorVBO);
		m_ParticleSimulationKernel = new OpenCLKernel(m_ParticleProgram, "ParticleSimulation",
			{
				new KernelArg("positionBuffer", m_CLPositionBuffer->GetBufferID(),	OpenCLBuffer::NativeSize(), KernelArgType::Global),
				new KernelArg("velocityBuffer", m_CLVelocityBuffer->GetBufferID(),	OpenCLBuffer::NativeSize(), KernelArgType::Global),
				new KernelArg("colorBuffer",	m_CLColorBuffer->GetBufferID(),		OpenCLBuffer::NativeSize(), KernelArgType::Global),
			});

		m_ParticleProgram->AddBuffer(m_CLColorBuffer);
		m_ParticleProgram->AddBuffer(m_CLVelocityBuffer);
		m_ParticleProgram->AddBuffer(m_CLPositionBuffer);
		m_ParticleProgram->AddKernel(m_ParticleSimulationKernel);
		m_ParticleSimulationKernel->AttachArgs();
	}

	void ParticleSystem::Tick(float dt)
	{
		m_ParticleProgram->EnqueueAcquireGLObjects("positionBuffer");
		m_ParticleProgram->EnqueueAcquireGLObjects("colorBuffer");
		m_ParticleProgram->Execute("ParticleSimulation", m_GlobalWorkSize, m_LocalWorkSize, 0);
		m_ParticleProgram->EnqueueReleaseGLObjects("positionBuffer");
		m_ParticleProgram->EnqueueReleaseGLObjects("colorBuffer");
	}

	void ParticleSystem::Render(const Camera& camera)
	{
		m_VAO->EnableVertexAttributes();
		m_ParticlePointShader->UploadUniformMat4("u_ViewProjectionMatrix", camera.GetViewProjection());
		m_ParticlePointShader->Bind();
		RenderCommand::DrawPoints(m_Properties.ParticleCount);
	}

	void ParticleSystem::Reset()
	{
		const glm::vec3 minExtents = m_Properties.Bounds.GetMinExtents();
		const glm::vec3 maxExtents = m_Properties.Bounds.GetMaxExtents();

		glm::vec4* positions = (glm::vec4*)m_ParticlePositionVBO->MapBufferRange(m_Properties.PositionDataByteSize, BufferHint::WriteOnly);
		for (int i = 0; i < m_Properties.ParticleCount; i++)
		{
			positions[i].x = Random::RandomRange(minExtents.x, maxExtents.x);
			positions[i].y = Random::RandomRange(minExtents.y, maxExtents.y);
			positions[i].z = Random::RandomRange(minExtents.z, maxExtents.z);
			positions[i].w = 1.0f;
		}
		m_ParticlePositionVBO->UnmapBuffer();

		glm::vec4* colors = (glm::vec4*)m_ParticleColorVBO->MapBufferRange(m_Properties.ColorDataByteSize, BufferHint::WriteOnly);
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
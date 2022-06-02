#include "glclpch.h"
#include "Particle/ParticleSystem.h"
#include "Engine/Renderer/RenderCommand.h"

#include "Engine/Input.h"

#include "Engine/Random.h"
#include <glm/glm.hpp>

#define PI 3.14159265359

namespace Engine
{
	ParticleSystem::ParticleSystem(const ParticleSystemProperties& properties, const std::string& clKernelFilePath, const std::string& shaderFilePath)
		:m_Properties(properties)
	{
		size_t dataSize = properties.ParticleCount * sizeof(float) * 4;
		m_Properties.ColorDataByteSize = m_Properties.PositionDataByteSize = m_Properties.VelocityDataByteSize = dataSize;

		m_GlobalWorkSize = glm::ivec3(properties.ParticleCount, 1, 1);
		m_LocalWorkSize = glm::ivec3(c_ThreadsPerWorkGroup, 1, 1);
		m_World = new SimulationWorld();

		m_World->AddSphere(glm::vec3(0.0f), 0.5f);

		m_World->AddSphere(glm::vec3( 0.30f, 0.0f,  0.0f), 0.10f);
		m_World->AddSphere(glm::vec3(-0.30f, 0.0f,  0.0f), 0.10f);
		m_World->AddSphere(glm::vec3( 0.0f, 0.0f,  0.30f), 0.10f);
		m_World->AddSphere(glm::vec3( 0.0f, 0.0f, -0.30f), 0.10f);

		m_World->AddSphere(glm::vec3( 0.22f, 0.0f, 0.22f), 0.10f);
		m_World->AddSphere(glm::vec3(-0.22f, 0.0f, 0.22f), 0.10f);
		m_World->AddSphere(glm::vec3( 0.22f, 0.0f, -0.22f), 0.10f);
		m_World->AddSphere(glm::vec3(-0.22f, 0.0f, -0.22f), 0.10f);

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
		delete[] m_SpheresPtr;
	}

	void ParticleSystem::UpdateBounds()
	{
		const SimulationBounds& bounds = m_World->GetBounds();
		glm::vec3 boundsCenter = bounds.GetCenter();
		cl_float4 center = { boundsCenter.x, boundsCenter.y, boundsCenter.z, 1.0f };

		glm::vec3 maxExtent = bounds.GetMaxExtents();
		cl_float4 max = { maxExtent.x, maxExtent.y, maxExtent.z, 1.0f };

		glm::vec3 minExtent = bounds.GetMinExtents();
		cl_float4 min = { minExtent.x, minExtent.y, minExtent.z, 1.0f };

		m_CLBoundsPtr = new cl_simulation_bounds();
		m_CLBoundsPtr->Center = center;
		m_CLBoundsPtr->MaxExtent = max;
		m_CLBoundsPtr->MinExtent = min;

		m_ParticleProgram->WriteToDeviceBufferFromHostBuffer("boundsBuffer", sizeof(cl_simulation_bounds), m_CLBoundsPtr);
	}

	void ParticleSystem::Initialize(const std::string& clKernelFilePath, const std::string& shaderFilePath)
	{
		m_ParticlePointShader = new Shader(shaderFilePath);
		m_VAO = new VertexArray;
		m_ParticlePositionVBO = new VertexBuffer(m_Properties.PositionDataByteSize);
		m_ParticlePositionVBO->SetLayout({ {"a_Position", ShaderDataType::Float4} });
		m_ParticleColorVBO = new VertexBuffer(m_Properties.ColorDataByteSize);
		m_ParticleColorVBO->SetLayout({ {"a_Color", ShaderDataType::Float4} });
		m_VAO->AddVertexBuffer(m_ParticlePositionVBO);
		m_VAO->AddVertexBuffer(m_ParticleColorVBO);
		m_Velocities.resize(m_Properties.ParticleCount);
	
		std::vector<SimulationSphere*> spheres = m_World->GetSpheres();
		m_SpheresPtr = (cl_float4*)calloc(spheres.size(), sizeof(cl_float4));

		for (int i = 0; i < spheres.size(); ++i)
		{
			glm::vec4 sphere = spheres[i]->Sphere;
			cl_float4 cl_sphere = { sphere.x, sphere.y, sphere.z, sphere.w };
			m_SpheresPtr[i] = cl_sphere;
		}

		m_ParticleProgram =			new OpenCLProgram(clKernelFilePath);
		m_CLVelocityBuffer =		new OpenCLBuffer(m_ParticleProgram, "velocityBuffer",	m_Properties.VelocityDataByteSize,	CLBufferType::ReadWrite);
		m_CLPositionBuffer =		new OpenCLBuffer(m_ParticleProgram, "positionBuffer",	m_Properties.PositionDataByteSize,	CLBufferType::ReadWrite, m_ParticlePositionVBO);
		m_CLColorBuffer =			new OpenCLBuffer(m_ParticleProgram, "colorBuffer",		m_Properties.ColorDataByteSize,		CLBufferType::ReadWrite, m_ParticleColorVBO);
		m_SimulationBoundsBuffer =	new OpenCLBuffer(m_ParticleProgram, "boundsBuffer",		sizeof(cl_simulation_bounds),		CLBufferType::ReadOnly);
		m_SpheresBuffer =			new OpenCLBuffer(m_ParticleProgram, "spheresBuffer",	sizeof(cl_float4) * spheres.size(), CLBufferType::ReadOnly);
		m_TimeBuffer =				new OpenCLBuffer(m_ParticleProgram, "time",				sizeof(cl_float),					CLBufferType::ReadOnly);

		const SimulationBounds& bounds = m_World->GetBounds();
		glm::vec3 boundsCenter = bounds.GetCenter();
		cl_float4 center = { boundsCenter.x, boundsCenter.y, boundsCenter.z, 1.0f };

		glm::vec3 maxExtent = bounds.GetMaxExtents();
		cl_float4 max = { maxExtent.x, maxExtent.y, maxExtent.z, 1.0f };

		glm::vec3 minExtent = bounds.GetMinExtents();
		cl_float4 min = { minExtent.x, minExtent.y, minExtent.z, 1.0f };

		m_CLBoundsPtr = new cl_simulation_bounds();
		m_CLBoundsPtr->Center = center;
		m_CLBoundsPtr->MaxExtent = max;
		m_CLBoundsPtr->MinExtent = min;

		m_TimePtr = new cl_float;
		*m_TimePtr = Time::Elapsed();

		m_ParticleSimulationKernel = new OpenCLKernel(m_ParticleProgram, "ParticleSimulation",
			{
				new KernelArg(m_CLPositionBuffer->GetBufferName(),			m_CLPositionBuffer->GetBufferID(),			OpenCLBuffer::NativeSize(),		KernelArgType::Global),
				new KernelArg(m_CLVelocityBuffer->GetBufferName(),			m_CLVelocityBuffer->GetBufferID(),			OpenCLBuffer::NativeSize(),		KernelArgType::Global),
				new KernelArg(m_CLColorBuffer->GetBufferName(),				m_CLColorBuffer->GetBufferID(),				OpenCLBuffer::NativeSize(),		KernelArgType::Global),
				new KernelArg(m_SimulationBoundsBuffer->GetBufferName(),	m_SimulationBoundsBuffer->GetBufferID(),	OpenCLBuffer::NativeSize(),		KernelArgType::Global),
				new KernelArg(m_SpheresBuffer->GetBufferName(),				m_SpheresBuffer->GetBufferID(),				OpenCLBuffer::NativeSize(),		KernelArgType::Global),
				new KernelArg(m_TimeBuffer->GetBufferName(),				m_TimeBuffer->GetBufferID(),				OpenCLBuffer::NativeSize(),		KernelArgType::Global),
			});

		m_ParticleProgram->AddBuffer(m_CLPositionBuffer);
		m_ParticleProgram->AddBuffer(m_CLVelocityBuffer);
		m_ParticleProgram->AddBuffer(m_CLColorBuffer);
		m_ParticleProgram->AddBuffer(m_SimulationBoundsBuffer);
		m_ParticleProgram->AddBuffer(m_SpheresBuffer);
		m_ParticleProgram->AddBuffer(m_TimeBuffer);
		m_ParticleProgram->AddKernel(m_ParticleSimulationKernel);
		m_ParticleSimulationKernel->AttachArgs();

		m_ParticleProgram->WriteToDeviceBufferFromHostBuffer("boundsBuffer", sizeof(cl_simulation_bounds), m_CLBoundsPtr);
		m_ParticleProgram->WriteToDeviceBufferFromHostBuffer("spheresBuffer", sizeof(cl_float4) * spheres.size(), m_SpheresPtr);
		m_ParticleProgram->WriteToDeviceBufferFromHostBuffer("time", sizeof(cl_float), m_TimePtr);

		m_PulseKernel = new OpenCLKernel(m_ParticleProgram, "ApplyPulse",
			{
				new KernelArg(m_CLPositionBuffer->GetBufferName(),			m_CLPositionBuffer->GetBufferID(),			OpenCLBuffer::NativeSize(),		KernelArgType::Global),
				new KernelArg(m_CLVelocityBuffer->GetBufferName(),			m_CLVelocityBuffer->GetBufferID(),			OpenCLBuffer::NativeSize(),		KernelArgType::Global),
				new KernelArg(m_SimulationBoundsBuffer->GetBufferName(),	m_SimulationBoundsBuffer->GetBufferID(),	OpenCLBuffer::NativeSize(),		KernelArgType::Global),
			});

		m_ParticleProgram->AddKernel(m_PulseKernel);
		m_PulseKernel->AttachArgs();
	}

	void ParticleSystem::Tick(float dt)
	{
		if (!m_Start) return;

		*m_TimePtr = Time::Elapsed();
		m_ParticleProgram->WriteToDeviceBufferFromHostBuffer("time", sizeof(cl_float), m_TimePtr);

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
		m_World->Render(camera.GetViewProjection());
	}

	static float cbrt(float n)
	{
		return std::pow(n, 1 / 3.0);
	}

	static glm::vec4 PointInSphere(const glm::vec3& center, float radius)
	{
		float u = Random::RandomRange(0.0f, 1.0f);
		float v = Random::RandomRange(0.0f, 1.0f);
		float theta = u * 2.0f * PI;
		float phi = glm::acos(2.0 * v - 1.0);
		float r = cbrt(Random::RandomRange(0.0f, 1.0f));
		float sinTheta = glm::sin(theta);
		float cosTheta = glm::cos(theta);
		float sinPhi = glm::sin(phi);
		float cosPhi = glm::cos(phi);
		float x = r * sinPhi * cosTheta * radius;
		float y = r * sinPhi * sinTheta * radius;
		float z = r * cosPhi * radius;
		return glm::vec4{ x, y, z, 1.0 };
	}

	void ParticleSystem::Reset()
	{
		m_Start = false;
		const SimulationBounds& bounds = m_World->GetBounds();
		float radius = abs(bounds.GetMaxExtents().x - bounds.GetMinExtents().x) / 4.0f - 0.5f;

		glm::vec4* positions = (glm::vec4*)m_ParticlePositionVBO->MapBuffer(m_Properties.PositionDataByteSize, BufferHint::WriteOnly);
		for (int i = 0; i < m_Properties.ParticleCount; i++)
			positions[i] = PointInSphere(bounds.GetCenter(), radius);
		m_ParticlePositionVBO->UnmapBuffer();

		glm::vec4* colors = (glm::vec4*)m_ParticleColorVBO->MapBuffer(m_Properties.ColorDataByteSize, BufferHint::WriteOnly);
		for (int i = 0; i < m_Properties.ParticleCount; i++)
			colors[i] = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
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

	void ParticleSystem::ApplyPulse()
	{
		m_ParticleProgram->EnqueueAcquireGLObjects("positionBuffer");
		m_ParticleProgram->Execute("ApplyPulse", m_GlobalWorkSize, m_LocalWorkSize, 0);
		m_ParticleProgram->Flush();
		m_ParticleProgram->EnqueueReleaseGLObjects("positionBuffer");
	}
}
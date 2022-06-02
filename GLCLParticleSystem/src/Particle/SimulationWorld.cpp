#include "glclpch.h"
#include "Particle/SimulationWorld.h"
#include "Engine/Renderer/RenderCommand.h"

namespace Engine
{
	SimulationWorld::SimulationWorld(const glm::vec3& center, const glm::vec3& size)
	{
		m_Bounds = new SimulationBounds(center, size);
		m_BoundsRenderer = new MeshRenderer(PrimitiveType::Box, center, size);
	}

	SimulationWorld::~SimulationWorld()
	{
		m_Spheres.clear();
		m_SphereMeshRenderers.clear();
		delete m_BoundsRenderer;
		delete m_Bounds;
	}

	void SimulationWorld::AddSphere(const glm::vec3& center, float radius)
	{
		SimulationSphere* sphere = new SimulationSphere();
		MeshRenderer* meshRenderer = new MeshRenderer(PrimitiveType::Sphere, center, glm::vec3(1.0f) * radius);
		sphere->Sphere = glm::vec4(center, radius * 0.5f);
		m_Spheres.push_back(sphere);
		m_SphereMeshRenderers.push_back(meshRenderer);
	}

	void SimulationWorld::Render(const glm::mat4& viewProjectionMatrix)
	{
		RenderCommand::SetDrawMode(DrawMode::WireFrame);

		if (m_RenderSpheres)
		{
			for (auto* sphere : m_SphereMeshRenderers)
				sphere->Render(viewProjectionMatrix);

			m_BoundsRenderer->Render(viewProjectionMatrix);
		}

	}

	void SimulationWorld::RotateBounds(float amount)
	{
		m_BoundsRenderer->Rotate(amount);
		m_Bounds->SetCenter(m_BoundsRenderer->GetModelMatrix() * glm::vec4(m_Bounds->GetCenter(), 1.0f));
		m_Bounds->SetMinExtents(m_BoundsRenderer->GetModelMatrix() * glm::vec4(m_Bounds->GetMinExtents(), 1.0f));
		m_Bounds->SetMaxExtents(m_BoundsRenderer->GetModelMatrix() * glm::vec4(m_Bounds->GetMaxExtents(), 1.0f));
	}
}
#pragma once

#include "Engine/Renderer/MeshRenderer.h"
#include "Particle/SimulationBounds.h"

#include <glm/glm.hpp>

namespace Engine
{
	struct SimulationBox
	{
		glm::vec3 Center;
		glm::vec3 Size;
		glm::vec3 MinExtents;
		glm::vec3 MaxExtents;
	};

	struct SimulationSphere
	{
		glm::vec4 Sphere;
	};

	class SimulationWorld
	{
	public:
		SimulationWorld(const glm::vec3& center = glm::vec3(0.0f), const glm::vec3& size = glm::vec3(1.0f));
		~SimulationWorld();

		const SimulationBounds& GetBounds() const { return *m_Bounds; }
		void AddSphere(const glm::vec3& center, float radius);
		void Render(const glm::mat4& viewProjectionMatrix);
		void RotateBounds(float amount);
		void ToggleRenderSpheres() { m_RenderSpheres = !m_RenderSpheres; }

		const std::vector<SimulationSphere*>& GetSpheres() const { return m_Spheres; }

		void RotateSpheres();

	private:
		bool m_RenderSpheres = true;
		MeshRenderer* m_BoundsRenderer;
		SimulationBounds* m_Bounds;
		std::vector<MeshRenderer*> m_SphereMeshRenderers;
		std::vector<SimulationSphere*> m_Spheres;
	};
}
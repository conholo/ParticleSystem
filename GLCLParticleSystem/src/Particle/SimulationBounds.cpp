#include "glclpch.h"
#include "Particle/SimulationBounds.h"

namespace Engine
{
	SimulationBounds::SimulationBounds(const glm::vec3& center, const glm::vec3& size)
		:m_Center(center), m_Size(size)
	{
		if (glm::dot(size, size) <= 0.0f)
			m_Size = glm::vec3(1.0f);

		m_MinExtents = m_Center - m_Size * 0.5f;
		m_MaxExtents = m_Center + m_Size * 0.5f;
	}

	void SimulationBounds::Move(const glm::vec3& newCenter)
	{
		m_Center = newCenter;
		m_MinExtents = m_Center - m_Size * 0.5f;
		m_MaxExtents = m_Center + m_Size * 0.5f;
	}

	void SimulationBounds::Resize(const glm::vec3& newSize)
	{
		m_Size = glm::dot(newSize, newSize) <= 0.0f ? glm::vec3(1.0f) : newSize;
		m_MinExtents = m_Center - m_Size * 0.5f;
		m_MaxExtents = m_Center + m_Size * 0.5f;
	}
}
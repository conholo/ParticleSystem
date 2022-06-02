#pragma once

#include <glm/glm.hpp>

namespace Engine
{
	class SimulationBounds
	{
	public:
		SimulationBounds(const glm::vec3& center = glm::vec3(0.0), const glm::vec3& size = glm::vec3(1.0));

		const glm::vec3& GetCenter() const { return m_Center; }
		const glm::vec3& GetSize() const { return m_Size; }
		const glm::vec3& GetMinExtents() const { return m_MinExtents; }
		const glm::vec3& GetMaxExtents() const { return m_MaxExtents; }

		void SetCenter(const glm::vec3& newCenter) { m_Center = newCenter; }
		void SetMinExtents(const glm::vec3& newMinExtents) { m_MinExtents = newMinExtents; }
		void SetMaxExtents(const glm::vec3& newMaxExtents) { m_MaxExtents = newMaxExtents; }

		void Move(const glm::vec3& newCenter);
		void Resize(const glm::vec3& newSize);

	private:
		glm::vec3 m_Center;
		glm::vec3 m_Size;
		glm::vec3 m_MinExtents;
		glm::vec3 m_MaxExtents;
	};
}
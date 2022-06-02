#pragma once

#include "Engine/Renderer/Shader.h"
#include "Engine/Renderer/VertexArray.h"
#include "Engine/Renderer/Mesh.h"

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Engine
{
	class MeshRenderer
	{
	public:
		MeshRenderer(PrimitiveType type, const glm::vec3& center = glm::vec3(0.0f), const glm::vec3& scale = glm::vec3(2.0f));
		~MeshRenderer();

		void Render(const glm::mat4& viewProjection);

		const glm::mat4& GetModelMatrix() const { return m_ModelMatrix; }
		const glm::quat& GetRotation() const;

		void Rotate(float amount);

	private:
		glm::vec3 m_Position = glm::vec3(0.0f);
		glm::vec3 m_Scale = glm::vec3(1.0f);
		glm::vec3 m_Rotation = glm::vec3(0.0f);
		glm::mat4 m_ModelMatrix;

		static Shader* s_FlatShader;

		VertexArray* m_VAO;
		VertexBuffer* m_VBO;
		IndexBuffer* m_EBO;

		Mesh* m_Mesh;
	};
}
#include "glclpch.h"

#include "Engine/Renderer/MeshRenderer.h"
#include "Engine/Renderer/RenderCommand.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>


namespace Engine
{

	Shader* MeshRenderer::s_FlatShader = nullptr;

	MeshRenderer::MeshRenderer(PrimitiveType type, const glm::vec3& center, const glm::vec3& scale)
		:m_Position(center), m_Scale(scale / 2.0f)
	{
		if (s_FlatShader == nullptr)
			s_FlatShader = new Shader("resources/shaders/flatcolor.shader");

		m_Mesh = MeshFactory::Create(type);
		m_ModelMatrix = glm::translate(glm::mat4(1.0f), m_Position) * glm::toMat4(glm::quat(m_Rotation)) * glm::scale(glm::mat4(1.0f), m_Scale);

		m_VAO = new VertexArray;
		m_VBO = new VertexBuffer((float*)m_Mesh->GetVertices().data(), sizeof(Vertex) * m_Mesh->GetVertices().size());
		m_EBO = new IndexBuffer((uint32_t*)m_Mesh->GetIndices().data(), m_Mesh->GetIndices().size());

		BufferLayout layout =
		{
			{ "a_Position", ShaderDataType::Float3 },
			{ "a_Normal",	ShaderDataType::Float3 },
		};

		m_VBO->SetLayout(layout);

		m_VAO->AddVertexBuffer(m_VBO);
		m_VAO->SetIndexBuffer(m_EBO);
	}

	MeshRenderer::~MeshRenderer()
	{
		delete m_VBO;
		delete m_EBO;
		delete m_VAO;
		delete m_Mesh;
		if (s_FlatShader != nullptr)
		{
			delete s_FlatShader;
			s_FlatShader = nullptr;
		}
	}

	void MeshRenderer::Render(const glm::mat4& viewProjection)
	{
		s_FlatShader->Bind();
		s_FlatShader->UploadUniformMat4("u_MVP", viewProjection * m_ModelMatrix);
		m_VAO->EnableVertexAttributes();
		RenderCommand::DrawIndexed(m_VAO);
	}

	const glm::quat& MeshRenderer::GetRotation() const
	{
		return glm::quat(m_Rotation);
	}

	void MeshRenderer::Rotate(float amount)
	{
		m_Rotation = glm::vec3(m_Rotation.x, m_Rotation.y, m_Rotation.z + amount);
		m_ModelMatrix = glm::translate(glm::mat4(1.0f), m_Position) * glm::toMat4(glm::quat(m_Rotation)) * glm::scale(glm::mat4(1.0f), m_Scale);
	}
}
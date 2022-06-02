#include "glclpch.h"
#include "Engine/Renderer/VertexArray.h"
#include "Engine/Renderer/BufferLayout.h"
#include <glad/glad.h>

namespace Engine
{
	GLenum GLEnumFromShaderDataType(ShaderDataType type)
	{
		switch (type)
		{
		case ShaderDataType::Mat3:
		case ShaderDataType::Mat4:
		case ShaderDataType::Float:
		case ShaderDataType::Float2:
		case ShaderDataType::Float3:
		case ShaderDataType::Float4:	return GL_FLOAT;
		case ShaderDataType::Int:		return GL_INT;
		default:						return GL_FLOAT;
		}
	}

	VertexArray::VertexArray()
	{
		glCreateVertexArrays(1, &m_ID);
		glBindVertexArray(m_ID);
	}

	VertexArray::~VertexArray()
	{
		glDeleteVertexArrays(1, &m_ID);
	}

	void VertexArray::Bind() const
	{
		glBindVertexArray(m_ID);
	}

	void VertexArray::Unbind() const
	{
		glBindVertexArray(0);
	}

	void VertexArray::SetIndexBuffer(IndexBuffer* indexBuffer)
	{
		glBindVertexArray(m_ID);
		indexBuffer->Bind();
		m_IndexBuffer = indexBuffer;
	}

	void VertexArray::AddVertexBuffer(VertexBuffer* vertexBuffer)
	{
		m_VBOs.push_back(vertexBuffer);
	}

	void VertexArray::EnableVertexAttributes()
	{
		uint32_t index = 0;
		uint32_t currentOffset = 0;

		for (int i = 0; i < m_VBOs.size(); i++)
		{
			VertexBuffer* vbo = m_VBOs[i];
			glBindVertexArray(m_ID);
			vbo->Bind();

			const auto& layout = vbo->GetLayout();

			for (const auto& element : layout)
			{
				glEnableVertexAttribArray(index);
				glVertexAttribPointer(
					index,
					element.GetComponentCount(),
					GLEnumFromShaderDataType(element.Type),
					element.Normalized ? GL_TRUE : GL_FALSE,
					layout.GetStride(),
					(void*)currentOffset
				);

				currentOffset += layout.GetStride();
				index++;
			}
		}
	}

}


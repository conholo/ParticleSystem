#pragma once

#include "Engine/Renderer/VertexBuffer.h"
#include "Engine/Renderer/IndexBuffer.h"
#include "Engine/Renderer/BufferLayout.h"

namespace Engine
{
	class VertexArray
	{
	public:
		VertexArray();
		~VertexArray();

		void Bind() const;
		void Unbind() const;

		void ClearIndexBuffer() { m_IndexBuffer = nullptr; }
		void SetIndexBuffer(IndexBuffer* indexBuffer);
		IndexBuffer* GetIndexBuffer() const { return m_IndexBuffer; }
		void AddVertexArray(VertexBuffer* vertexBuffer);
		void EnableVertexAttributes();

	private:
		std::vector<VertexBuffer*> m_VBOs;
		IndexBuffer* m_IndexBuffer;
		uint32_t m_ID;
	};
}


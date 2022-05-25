#pragma once

#include "Engine/Renderer/VertexBuffer.h"
#include "Engine/Renderer/IndexBuffer.h"

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
		void EnableVertexAttributes(VertexBuffer* vertexBuffer);

	private:
		IndexBuffer* m_IndexBuffer;
		uint32_t m_ID;
	};
}


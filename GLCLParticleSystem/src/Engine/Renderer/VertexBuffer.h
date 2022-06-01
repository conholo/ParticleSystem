#pragma once

#include "Engine/Renderer/BufferLayout.h"

namespace Engine
{
	class VertexBuffer
	{
	public:
		VertexBuffer(uint32_t size);
		VertexBuffer(float* vertices, uint32_t size);

		~VertexBuffer();

		void* MapBuffer(uint32_t size, BufferHint hint);
		void UnmapBuffer();
		void SetData(const void* data, uint32_t size);
		void Resize(uint32_t size);
		void ResizeAndSetData(const void* data, uint32_t size);

		void Bind() const;
		void Unbind() const;

		uint32_t GetID() const { return m_ID; }
		void SetLayout(const BufferLayout& layout) { m_Layout = layout; }
		const BufferLayout& GetLayout() const { return m_Layout; }

	private:
		size_t m_Size;
		uint32_t m_ID = 0;
		BufferLayout m_Layout;
	};
}


#include "glclpch.h"
#include "Engine/Renderer/VertexBuffer.h"
#include <glad/glad.h>

namespace Engine
{
	static GLenum GLHintFromBufferHint(BufferHint hint)
	{
		switch (hint)
		{
		case BufferHint::ReadOnly: return GL_READ_ONLY;
		case BufferHint::WriteOnly: return GL_WRITE_ONLY;
		case BufferHint::ReadWrite: return GL_READ_WRITE;
		}

		LOG_ERROR("Invalid buffer hint for VBO.");
		return -1;
	}

	VertexBuffer::VertexBuffer(uint32_t size)
		:m_Size(size)
	{
		glCreateBuffers(1, &m_ID);
		glBindBuffer(GL_ARRAY_BUFFER, m_ID);
		glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
	}

	VertexBuffer::VertexBuffer(float* vertices, uint32_t size)
		:m_Size(size)
	{
		glCreateBuffers(1, &m_ID);
		glBindBuffer(GL_ARRAY_BUFFER, m_ID);
		glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
	}

	VertexBuffer::~VertexBuffer()
	{
		glDeleteBuffers(1, &m_ID);
	}

	void* VertexBuffer::MapBuffer(uint32_t size, BufferHint hint)
	{
		if (m_Size != size)
		{
			LOG_ERROR("Attempting to map buffer with invalid size. Expected: {} - Requested: {}.", m_Size, size);
			return nullptr;
		}

		GLenum bufferHint = GLHintFromBufferHint(hint);
		glBindBuffer(GL_ARRAY_BUFFER, m_ID);
		void* buffer = glMapNamedBuffer(m_ID, bufferHint);
		return buffer;
	}

	void VertexBuffer::UnmapBuffer()
	{
		glUnmapNamedBuffer(m_ID);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void VertexBuffer::SetData(const void* data, uint32_t size)
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_ID);
		glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
	}

	void VertexBuffer::Resize(uint32_t size)
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_ID);
		glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
	}

	void VertexBuffer::ResizeAndSetData(const void* data, uint32_t size)
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_ID);
		glBufferData(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW);
	}

	void VertexBuffer::Bind() const
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_ID);
	}

	void VertexBuffer::Unbind() const
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
}

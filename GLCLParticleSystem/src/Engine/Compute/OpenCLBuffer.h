#pragma once

#include <OpenCL/cl.h>
#include <OpenCL/cl_platform.h>
#include "Engine/Renderer/VertexBuffer.h"

namespace Engine
{
	class OpenCLProgram;

	enum class CLBufferType { None, WriteOnly, ReadOnly, ReadWrite };

	class OpenCLBuffer
	{
	public:
		OpenCLBuffer(OpenCLProgram* program, const std::string& bufferName, size_t dataSize, CLBufferType type);
		OpenCLBuffer(OpenCLProgram* program, const std::string& bufferName, size_t dataSize, CLBufferType type, VertexBuffer* vbo);
		~OpenCLBuffer();

		static size_t NativeSize() { return sizeof(cl_mem); }

		bool IsAttachedToGLBuffer() const { return m_AttachedVBO != nullptr; }

		size_t GetBufferSize() const { return m_DataSize; }
		const std::string& GetBufferName() const { return m_BufferName; }
		cl_mem GetBufferID() const { return m_BufferID; }
		CLBufferType GetType() const { return m_Type; }

	private:
		VertexBuffer* m_AttachedVBO = nullptr;
		CLBufferType m_Type;
		OpenCLProgram* m_Program;
		std::string m_BufferName;
		size_t m_DataSize;
		cl_mem m_BufferID;
	};
}
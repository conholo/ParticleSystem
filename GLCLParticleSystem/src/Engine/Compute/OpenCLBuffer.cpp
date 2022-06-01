#include "glclpch.h"
#include "Engine/Compute/OpenCLBuffer.h"
#include "Engine/Compute/OpenCLContext.h"
#include "Engine/Compute/OpenCLProgram.h"

#include <OpenCL/cl_gl.h>
#include <OpenCL/cl_gl_ext.h>

namespace Engine
{

	static cl_mem_flags CLFlagsFromBufferType(CLBufferType bufferType)
	{
		switch (bufferType)
		{
		case CLBufferType::ReadOnly: return CL_MEM_READ_ONLY;
		case CLBufferType::WriteOnly: return CL_MEM_WRITE_ONLY;
		case CLBufferType::ReadWrite: return CL_MEM_READ_WRITE;
		}

		LOG_ERROR("Invalid CLBufferType.");
		return -1;
	}

	OpenCLBuffer::OpenCLBuffer(OpenCLProgram* program, const std::string& bufferName, size_t dataSize, CLBufferType bufferType)
		:m_Program(program), m_BufferName(bufferName), m_DataSize(dataSize), m_Type(bufferType)
	{
		cl_int status;
		cl_mem_flags type = CLFlagsFromBufferType(bufferType);
		m_BufferID = clCreateBuffer(OpenCLContext::GetContext(), type, dataSize, NULL, &status);
		OpenCLContext::PrintCLError(status, "clCreateBuffer failed (1)");
	}

	OpenCLBuffer::OpenCLBuffer(OpenCLProgram* program, const std::string& bufferName, size_t dataSize, CLBufferType bufferType, VertexBuffer* vbo)
		:m_Program(program), m_BufferName(bufferName), m_DataSize(dataSize), m_Type(bufferType), m_AttachedVBO(vbo)
	{
		cl_int status;
		cl_mem_flags type = CLFlagsFromBufferType(bufferType);
		m_BufferID = clCreateFromGLBuffer(OpenCLContext::GetContext(), type, vbo->GetID(), &status);
		OpenCLContext::PrintCLError(status, "clCreateFromGLBuffer failed (1)");
	}

	OpenCLBuffer::~OpenCLBuffer()
	{
		clReleaseMemObject(m_BufferID);
	}
}
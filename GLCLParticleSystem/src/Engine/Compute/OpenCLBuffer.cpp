#include "glclpch.h"
#include "Engine/Compute/OpenCLBuffer.h"
#include "Engine/Compute/OpenCLContext.h"
#include "Engine/Compute/OpenCLProgram.h"


namespace Engine
{
	OpenCLBuffer::OpenCLBuffer(OpenCLProgram* program, const std::string& bufferName, size_t dataSize, CLBufferType bufferType)
		:m_Program(program), m_BufferName(bufferName), m_DataSize(dataSize), m_Type(bufferType)
	{
		cl_int status;
		cl_mem_flags type = m_Type == CLBufferType::ReadOnly ? CL_MEM_READ_ONLY : CL_MEM_WRITE_ONLY;
		m_BufferID = clCreateBuffer(OpenCLContext::GetContext(), type, dataSize, NULL, &status);
		if (status != CL_SUCCESS)
			LOG_ERROR("clCreateBuffer failed (1)");

		OpenCLContext::Wait(program->GetCommandQueueID());
	}

	OpenCLBuffer::~OpenCLBuffer()
	{
		clReleaseMemObject(m_BufferID);
	}
}
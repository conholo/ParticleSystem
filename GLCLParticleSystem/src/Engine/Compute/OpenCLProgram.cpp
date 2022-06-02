#include "glclpch.h"
#include "Engine/Compute/OpenCLProgram.h"
#include "Engine/Compute/OpenCLContext.h"
#include <OpenCL/cl_gl.h>
#include <OpenCL/cl_gl_ext.h>


namespace Engine
{
	OpenCLProgram::OpenCLProgram(const std::string& source)
	{
		FILE* fp;
		errno_t err = fopen_s(&fp, source.c_str(), "r");
		if (err != 0)
			LOG_ERROR("Cannot open OpenCL source file: {}", source);

		fseek(fp, 0, SEEK_END);
		size_t fileSize = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		char* clProgramText = new char[fileSize + 1];		// leave room for '\0'
		size_t n = fread(clProgramText, 1, fileSize, fp);
		clProgramText[fileSize] = '\0';
		fclose(fp);
		if (n != fileSize)
			LOG_ERROR("Expected to read {} bytes read from {} -- actually read {}.", fileSize, source.c_str(), n);

		char* strings[1];
		strings[0] = clProgramText;

		cl_int status;

		m_ID = clCreateProgramWithSource(OpenCLContext::GetContext(), 1, (const char**)strings, NULL, &status);
		OpenCLContext::PrintCLError(status, "clCreateProgramWithSource failed");
		delete[] clProgramText;

		char* options = { "" };
		const cl_device_id id = OpenCLContext::GetDeviceRef();
		status = clBuildProgram(m_ID, 1, &id, options, NULL, NULL);
		if (status != CL_SUCCESS)
		{
			size_t size;
			clGetProgramBuildInfo(m_ID, OpenCLContext::GetDevice(), CL_PROGRAM_BUILD_LOG, 0, NULL, &size);
			cl_char* log = new cl_char[size];
			clGetProgramBuildInfo(m_ID, OpenCLContext::GetDevice(), CL_PROGRAM_BUILD_LOG, size, log, NULL);
			LOG_ERROR("clBuildProgram failed:\n{}", log);
			delete[] log;
		}


		m_CommandQueue = clCreateCommandQueue(OpenCLContext::GetContext(), OpenCLContext::GetDevice(), 0, &status);
		OpenCLContext::PrintCLError(status, "clCreateCommandQueue failed");
	}

	OpenCLProgram::~OpenCLProgram()
	{
		for (auto kernelEntry : m_Kernels)
			delete kernelEntry.second;

		for (auto bufferEntry : m_Buffers)
			delete bufferEntry.second;
	}

	void OpenCLProgram::AddKernel(const std::string& kernelName, const std::initializer_list<KernelArg*>& args)
	{
		if (OpenCLContext::GetShouldLogDebug() && m_Kernels.find(kernelName) != m_Kernels.end())
		{
			LOG_ERROR("Kernel with name: '{}' already exists in OpenCL Program.", kernelName);
			return;
		}
		m_Kernels[kernelName] = new OpenCLKernel(this, kernelName, args);
	}

	void OpenCLProgram::AddKernel(OpenCLKernel* kernel)
	{
		if (OpenCLContext::GetShouldLogDebug() && m_Kernels.find(kernel->GetKernelName()) != m_Kernels.end())
		{
			LOG_ERROR("Kernel with name: '{}' already exists in OpenCL Program.", kernel->GetKernelName());
			return;
		}

		m_Kernels[kernel->GetKernelName()] = kernel;
	}

	void OpenCLProgram::Execute(const std::string& kernelName, glm::ivec3& globalWorkSize, const glm::vec3& localWorkSize, uint32_t eventsInWaitListCount)
	{
		if (OpenCLContext::GetShouldLogDebug() && m_Kernels.find(kernelName) == m_Kernels.end())
		{
			LOG_ERROR("Unable to Execute CLProgram.  No kernel with name: {} found.", kernelName);
			return;
		}

		OpenCLKernel* kernel = m_Kernels[kernelName];
		kernel->AttachArgs();
		const size_t globalWorkSizes[3] = { globalWorkSize.x, globalWorkSize.y, globalWorkSize.z };
		const size_t lobalWorkSizes[3] = { localWorkSize.x, localWorkSize.y, localWorkSize.z };

		std::chrono::duration<double> elapsed;
		auto start = std::chrono::high_resolution_clock::now();
		cl_event wait;
		cl_int status = clEnqueueNDRangeKernel(m_CommandQueue, kernel->GetID(), 1, NULL, globalWorkSizes, lobalWorkSizes, 0, NULL, &wait);
		auto end = std::chrono::high_resolution_clock::now();
		elapsed = end - start;

		m_SumTimeMS += elapsed.count();
	}

	OpenCLBuffer* OpenCLProgram::GetBuffer(const std::string& bufferName)
	{
		if (m_Buffers.find(bufferName) == m_Buffers.end())
		{
			LOG_ERROR("Unable to retrieve buffer.  No buffer with name: {} found.", bufferName);
			return nullptr;
		}

		return m_Buffers[bufferName];
	}

	void OpenCLProgram::AddBuffer(const std::string& bufferName, size_t bufferSize, CLBufferType bufferType)
	{
		if (m_Buffers.find(bufferName) != m_Buffers.end())
		{
			LOG_ERROR("Buffer with name: '{}' already exists in OpenCL Program.", bufferName);
			return;
		}
		m_Buffers[bufferName] = new OpenCLBuffer(this, bufferName, bufferSize, bufferType);
	}

	void OpenCLProgram::AddBuffer(OpenCLBuffer* buffer)
	{
		if (m_Buffers.find(buffer->GetBufferName()) != m_Buffers.end())
		{
			LOG_ERROR("Buffer with name: '{}' already exists in OpenCL Program.", buffer->GetBufferName());
			return;
		}

		m_Buffers[buffer->GetBufferName()] = buffer;
	}

	void OpenCLProgram::ReadDeviceBufferToHostBuffer(const std::string& bufferName, size_t hostBufferSize, void* destinationBuffer)
	{
		if (m_Buffers.find(bufferName) == m_Buffers.end())
		{
			LOG_ERROR("Unable to Execute CLProgram.  No buffer with name: {} found.", bufferName);
			return;
		}

		OpenCLBuffer* buffer = m_Buffers[bufferName];

		if (buffer->GetType() == CLBufferType::WriteOnly)
		{
			LOG_WARN("Buffer: {} is flagged as WriteOnly.  Unexpected behavior may occur when attempting to read write-only device buffer.", bufferName);
		}

		if (hostBufferSize != buffer->GetBufferSize())
		{
			LOG_ERROR("Failed to copy device buffer to host.  Expected host buffer size: {}.  Given size: {}", buffer->GetBufferSize(), hostBufferSize);
			return;
		}

		cl_int status = clEnqueueReadBuffer(m_CommandQueue, buffer->GetBufferID(), CL_TRUE, 0, buffer->GetBufferSize(), destinationBuffer, 0, NULL, NULL);
		OpenCLContext::PrintCLError(status, "clEnqueueReadBuffer failed");
	}

	void OpenCLProgram::EnqueueAcquireGLObjects(const std::string& deviceBufferName)
	{
		OpenCLBuffer* deviceBuffer = GetBuffer(deviceBufferName);
		if (deviceBuffer == nullptr)
		{
			LOG_ERROR("Could not EnqueueAquireGLObjects with device buffer named: {} - buffer not found", deviceBufferName);
			return;
		}
		if (!deviceBuffer->IsAttachedToGLBuffer())
		{
			LOG_ERROR("Could not EnqueueAquireGLObjects with device buffer named: {} because this buffer is not associated with a GL buffer.", deviceBufferName);
			return;
		}

		cl_mem id = deviceBuffer->GetBufferID();
		cl_int status = clEnqueueAcquireGLObjects(m_CommandQueue, 1, &id, 0, NULL, NULL);
		OpenCLContext::PrintCLError(status, "clEnqueueAcquireGLObjects failed");
	}

	void OpenCLProgram::EnqueueReleaseGLObjects(const std::string& deviceBufferName)
	{
		OpenCLBuffer* deviceBuffer = GetBuffer(deviceBufferName);
		if (deviceBuffer == nullptr)
		{
			LOG_ERROR("Could not EnqueueReleaseGLObjects with device buffer named: {} - buffer not found.", deviceBufferName);
			return;
		}
		if (!deviceBuffer->IsAttachedToGLBuffer())
		{
			LOG_ERROR("Could not EnqueueReleaseGLObjects with device buffer named: {} because this buffer is not associated with a GL buffer.", deviceBufferName);
			return;
		}

		cl_mem id = deviceBuffer->GetBufferID();
		cl_int status = clEnqueueReleaseGLObjects(m_CommandQueue, 1, &id, 0, NULL, NULL);
		OpenCLContext::PrintCLError(status, "clEnqueueReleaseGLObjects failed");
	}

	void OpenCLProgram::Flush()
	{
		clFinish(m_CommandQueue);
	}

	void OpenCLProgram::WriteToDeviceBufferFromHostBuffer(const std::string& deviceBufferName, size_t hostBufferSize, void* hostBuffer)
	{
		if (m_Buffers.find(deviceBufferName) == m_Buffers.end())
		{
			LOG_ERROR("Unable to write to device buffer from host.  No buffer with name: {} found.", deviceBufferName);
			return;
		}

		OpenCLBuffer* buffer = m_Buffers[deviceBufferName];

		cl_int status = clEnqueueWriteBuffer(m_CommandQueue, buffer->GetBufferID(), CL_FALSE, 0, hostBufferSize, hostBuffer, 0, NULL, NULL);
		if (status != CL_SUCCESS)
			LOG_ERROR("clEnqueueWriteBuffer failed (1)");
	}
}
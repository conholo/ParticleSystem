#pragma once

#include <OpenCL/cl.h>
#include <OpenCL/cl_platform.h>
#include "Engine/Compute/OpenCLKernel.h"
#include "Engine/Compute/OpenCLBuffer.h"

namespace Engine
{
	enum class BufferType { Read, Write };

	class OpenCLProgram
	{
	public:
		OpenCLProgram(const std::string& kernelFilePath);
		~OpenCLProgram();

		void AddKernel(const std::string& kernelName, const std::initializer_list<KernelArg*>& args);
		void AddBuffer(const std::string& bufferName, size_t bufferSize, CLBufferType bufferType);
		void ReadDeviceBufferToHostBuffer(const std::string& bufferName, size_t hostBufferSize, void* destinationBuffer);

		void WriteToDeviceBufferFromHostBuffer(const std::string& deviceBufferName, size_t hostBufferSize, void* hostBuffer);

		void Execute(const std::string& kernelName, glm::ivec3& globalWorkSize, const glm::vec3& localWorkSize, uint32_t eventsInWaitListCount);

		OpenCLBuffer* GetBuffer(const std::string& bufferName);

		cl_command_queue GetCommandQueueID() const { return m_CommandQueue; }

		cl_program GetID() const { return m_ID; }

	private:
		std::unordered_map<std::string, OpenCLKernel*> m_Kernels;
		std::unordered_map<std::string, OpenCLBuffer*> m_Buffers;
		cl_command_queue m_CommandQueue;
		cl_program m_ID;
	};
}
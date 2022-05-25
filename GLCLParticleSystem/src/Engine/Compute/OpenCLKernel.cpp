#include "glclpch.h"
#include "Engine/Compute/OpenCLKernel.h"
#include "Engine/Compute/OpenCLProgram.h"
#include "Engine/Compute/OpenCLContext.h"

namespace Engine
{
	OpenCLKernel::OpenCLKernel(Engine::OpenCLProgram* program, const std::string& kernelName, const std::initializer_list<KernelArg*>& args)
		:m_KernelName(kernelName), m_Program(program), m_Args(args)
	{
		cl_int status;
		m_KernelID = clCreateKernel(program->GetID(), kernelName.c_str(), &status);
		if (status != CL_SUCCESS)
			LOG_ERROR("clCreateKernel failed");
	}

	OpenCLKernel::~OpenCLKernel()
	{
		for (auto arg : m_Args)
		{
			if (!arg) continue;
			delete arg;
		}
	}

	void OpenCLKernel::AttachArgs()
	{
		cl_int status;

		for (int i = 0; i < m_Args.size(); i++)
		{
			KernelArg& arg = *m_Args[i];
			
			status = clSetKernelArg(m_KernelID, i, arg.Size, arg.Type == KernelArgType::Global ? &arg.Data : NULL);
			if (status != CL_SUCCESS)
				LOG_ERROR("Failure to set clSetKernelArg for Arg: {} at {}.", arg.Name, i);
		}

		OpenCLContext::Wait(m_Program->GetCommandQueueID());
	}
}
#pragma once

#include <OpenCL/cl.h>
#include <OpenCL/cl_platform.h>

namespace Engine
{
	class OpenCLProgram;

	enum class KernelArgType { None, Global, Local };

	struct KernelArg
	{
		KernelArg(const std::string& name, void* data, size_t size, KernelArgType argType)
			:Name(name), Data(data), Size(size), Type(argType) { }

		std::string Name;
		void* Data;
		size_t Size;
		KernelArgType Type;
	};

	class OpenCLKernel
	{
	public:
		OpenCLKernel(OpenCLProgram* program, const std::string& kernelName, const std::initializer_list<KernelArg*>& args);
		~OpenCLKernel();

		const std::string& GetKernelName() const { return m_KernelName; }
		cl_kernel GetID() const { return m_KernelID; }
		void AttachArgs();

	private:
		std::vector<KernelArg*> m_Args;
		std::string m_KernelName;
		cl_kernel m_KernelID;
		OpenCLProgram* m_Program;
	};
}
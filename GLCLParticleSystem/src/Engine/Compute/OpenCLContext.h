#pragma once

#include <OpenCL/cl.h>
#include <OpenCL/cl_platform.h>

namespace Engine
{
	class OpenCLContext
	{
	public:
		static void Initialize();

		static void SelectOpenCLDevice();
		static void Wait(cl_command_queue queue);
		static int BitCheck(float fp);

		static cl_platform_id GetPlatform() { return s_Platform; }
		static cl_device_id GetDevice() { return s_Device; }
		static cl_context GetContext() { return s_Context; }

		static cl_device_id& GetDeviceRef() { return s_Device; }

	private:

		static cl_platform_id s_Platform;
		static cl_device_id s_Device;
		static cl_context s_Context;
	};
}
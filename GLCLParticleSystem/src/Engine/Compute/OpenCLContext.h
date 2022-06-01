#pragma once

#include <OpenCL/cl.h>
#include <OpenCL/cl_platform.h>
#include <OpenCL/cl_gl_ext.h>
#include <OpenCL/cl_gl.h>

namespace Engine
{
	class OpenCLContext
	{
	public:
		static void Initialize();

		static void SelectOpenCLDevice();
		static void Wait(cl_command_queue queue);
		static int BitCheck(float fp);

		static void PrintCLError(cl_int errorCode, char* prefix);

		static cl_platform_id GetPlatform() { return s_Platform; }
		static cl_device_id GetDevice() { return s_Device; }
		static cl_context GetContext() { return s_Context; }

		static cl_device_id& GetDeviceRef() { return s_Device; }
		static void ToggleDebug(bool debug) { s_Debug = debug; }
		static bool GetShouldLogDebug() { return s_Debug; }
		static bool IsCLExtensionSupported(const char* extension);

	private:

		static bool s_Debug;
		static cl_platform_id s_Platform;
		static cl_device_id s_Device;
		static cl_context s_Context;
	};
}
#include "glclpch.h"
#include "Engine/Compute/OpenCLContext.h"

#ifdef _WIN32
#include <wingdi.h>
#include <windows.h>
#pragma warning(disable:4996)
#endif

#define ID_AMD          0x1002
#define ID_INTEL        0x8086
#define ID_NVIDIA       0x10de


namespace Engine
{
	cl_device_id OpenCLContext::s_Device = nullptr;
	cl_platform_id OpenCLContext::s_Platform = nullptr;
	cl_context OpenCLContext::s_Context = nullptr;
	bool OpenCLContext::s_Debug = true;

	struct errorcode
	{
		cl_int		statusCode;
		char* meaning;
	}
	ErrorCodes[] =
	{
		{ CL_SUCCESS,				""					},
		{ CL_DEVICE_NOT_FOUND,					"Device Not Found"			},
		{ CL_DEVICE_NOT_AVAILABLE,				"Device Not Available"			},
		{ CL_COMPILER_NOT_AVAILABLE,			"Compiler Not Available"		},
		{ CL_MEM_OBJECT_ALLOCATION_FAILURE,		"Memory Object Allocation Failure"	},
		{ CL_OUT_OF_RESOURCES,					"Out of resources"			},
		{ CL_OUT_OF_HOST_MEMORY,				"Out of Host Memory"			},
		{ CL_PROFILING_INFO_NOT_AVAILABLE,		"Profiling Information Not Available"	},
		{ CL_MEM_COPY_OVERLAP,					"Memory Copy Overlap"			},
		{ CL_IMAGE_FORMAT_MISMATCH,				"Image Format Mismatch"			},
		{ CL_IMAGE_FORMAT_NOT_SUPPORTED,		"Image Format Not Supported"		},
		{ CL_BUILD_PROGRAM_FAILURE,				"Build Program Failure"			},
		{ CL_MAP_FAILURE,						"Map Failure"				},
		{ CL_INVALID_VALUE,						"Invalid Value"				},
		{ CL_INVALID_DEVICE_TYPE,				"Invalid Device Type"			},
		{ CL_INVALID_PLATFORM,					"Invalid Platform"			},
		{ CL_INVALID_DEVICE,					"Invalid Device"			},
		{ CL_INVALID_CONTEXT,					"Invalid Context"			},
		{ CL_INVALID_QUEUE_PROPERTIES,			"Invalid Queue Properties"		},
		{ CL_INVALID_COMMAND_QUEUE,				"Invalid Command Queue"			},
		{ CL_INVALID_HOST_PTR,					"Invalid Host Pointer"			},
		{ CL_INVALID_MEM_OBJECT,				"Invalid Memory Object"			},
		{ CL_INVALID_IMAGE_FORMAT_DESCRIPTOR,	"Invalid Image Format Descriptor"	},
		{ CL_INVALID_IMAGE_SIZE,				"Invalid Image Size"			},
		{ CL_INVALID_SAMPLER,					"Invalid Sampler"			},
		{ CL_INVALID_BINARY,					"Invalid Binary"			},
		{ CL_INVALID_BUILD_OPTIONS,				"Invalid Build Options"			},
		{ CL_INVALID_PROGRAM,					"Invalid Program"			},
		{ CL_INVALID_PROGRAM_EXECUTABLE,		"Invalid Program Executable"		},
		{ CL_INVALID_KERNEL_NAME,				"Invalid Kernel Name"			},
		{ CL_INVALID_KERNEL_DEFINITION,			"Invalid Kernel Definition"		},
		{ CL_INVALID_KERNEL,					"Invalid Kernel"			},
		{ CL_INVALID_ARG_INDEX,					"Invalid Argument Index"		},
		{ CL_INVALID_ARG_VALUE,					"Invalid Argument Value"		},
		{ CL_INVALID_ARG_SIZE,					"Invalid Argument Size"			},
		{ CL_INVALID_KERNEL_ARGS,				"Invalid Kernel Arguments"		},
		{ CL_INVALID_WORK_DIMENSION,			"Invalid Work Dimension"		},
		{ CL_INVALID_WORK_GROUP_SIZE,			"Invalid Work Group Size"		},
		{ CL_INVALID_WORK_ITEM_SIZE,			"Invalid Work Item Size"		},
		{ CL_INVALID_GLOBAL_OFFSET,				"Invalid Global Offset"			},
		{ CL_INVALID_EVENT_WAIT_LIST,			"Invalid Event Wait List"		},
		{ CL_INVALID_EVENT,						"Invalid Event"				},
		{ CL_INVALID_OPERATION,					"Invalid Operation"			},
		{ CL_INVALID_GL_OBJECT,					"Invalid GL Object"			},
		{ CL_INVALID_BUFFER_SIZE,				"Invalid Buffer Size"			},
		{ CL_INVALID_MIP_LEVEL,					"Invalid MIP Level"			},
		{ CL_INVALID_GLOBAL_WORK_SIZE,			"Invalid Global Work Size"		},
	};


	bool OpenCLContext::IsCLExtensionSupported(const char* extension)
	{
		// see if the extension is bogus:

		if (extension == NULL || extension[0] == '\0')
			return false;

		char* where = (char*)strchr(extension, ' ');
		if (where != NULL)
			return false;

		// get the full list of extensions:

		size_t extensionSize;
		clGetDeviceInfo(s_Device, CL_DEVICE_EXTENSIONS, 0, NULL, &extensionSize);
		char* extensions = new char[extensionSize];
		clGetDeviceInfo(s_Device, CL_DEVICE_EXTENSIONS, extensionSize, extensions, NULL);

		for (char* start = extensions; ; )
		{
			where = (char*)strstr((const char*)start, extension);
			if (where == 0)
			{
				delete[] extensions;
				return false;
			}

			char* terminator = where + strlen(extension);	// points to what should be the separator

			if (*terminator == ' ' || *terminator == '\0' || *terminator == '\r' || *terminator == '\n')
			{
				delete[] extensions;
				return true;
			}
			start = terminator;
		}

		delete[] extensions;
		return false;
	}


	void OpenCLContext::Initialize()
	{
		SelectOpenCLDevice();

		if (IsCLExtensionSupported("cl_khr_gl_sharing"))
		{
			LOG_TRACE("cl_khr_gl_sharing is supported.");
		}
		else
		{
			LOG_CRITICAL("cl_khr_gl_sharing is not supported -- aborting.");
			return;
		}

		cl_int status;
		cl_context_properties props[] =
		{
			CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(),
			CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),
			CL_CONTEXT_PLATFORM, (cl_context_properties)s_Platform,
			0
		};

		s_Context = clCreateContext(props, 1, &s_Device, NULL, NULL, &status);
		PrintCLError(status, "clCreateContext failed");
	}

	static char* Vendor(cl_uint v)
	{
		switch (v)
		{
		case ID_AMD:
			return (char*)"AMD";
		case ID_INTEL:
			return (char*)"Intel";
		case ID_NVIDIA:
			return (char*)"NVIDIA";
		}
		return (char*)"Unknown";
	}

	static char* Type(cl_device_type t)
	{
		switch (t)
		{
		case CL_DEVICE_TYPE_CPU:
			return (char*)"CL_DEVICE_TYPE_CPU";
		case CL_DEVICE_TYPE_GPU:
			return (char*)"CL_DEVICE_TYPE_GPU";
		case CL_DEVICE_TYPE_ACCELERATOR:
			return (char*)"CL_DEVICE_TYPE_ACCELERATOR";
		}
		return (char*)"Unknown";
	}

	void OpenCLContext::Wait(cl_command_queue queue)
	{
		cl_event wait;
		cl_int status;

		status = clEnqueueMarker(queue, &wait);
		if (status != CL_SUCCESS)
			LOG_ERROR("Wait: clEnqueueMarker failed!");

		status = clWaitForEvents(1, &wait);
		if (status != CL_SUCCESS)
			LOG_ERROR("Wait: clWaitForEvents failed!");
	}

	int OpenCLContext::BitCheck(float fp)
	{
		int* ip = (int*)&fp;
		return *ip;
	}

	void OpenCLContext::PrintCLError(cl_int errorCode, char* prefix)
	{
		if (!s_Debug) return;

		if (errorCode == CL_SUCCESS)
			return;

		const int numErrorCodes = sizeof(ErrorCodes) / sizeof(struct errorcode);
		char* meaning = "";
		for (int i = 0; i < numErrorCodes; i++)
		{
			if (errorCode == ErrorCodes[i].statusCode)
			{
				meaning = ErrorCodes[i].meaning;
				break;
			}
		}

		LOG_CRITICAL("CL Error: {}: {}", prefix, meaning);
	}

	void OpenCLContext::SelectOpenCLDevice()
	{
		int bestPlatform = -1;
		int bestDevice = -1;
		cl_device_type bestDeviceType;
		cl_uint bestDeviceVendor;
		cl_int status;

		cl_uint numPlatforms;
		status = clGetPlatformIDs(0, NULL, &numPlatforms);
		if (status != CL_SUCCESS)
			LOG_CRITICAL("clGetPlatformIDs failed (1)");

		cl_platform_id* platforms = new cl_platform_id[numPlatforms];
		status = clGetPlatformIDs(numPlatforms, platforms, NULL);
		if (status != CL_SUCCESS)
			LOG_CRITICAL("clGetPlatformIDs failed (2)");

		for (int p = 0; p < (int)numPlatforms; p++)
		{
			// find out how many devices are attached to each platform and get their ids:

			cl_uint numDevices;

			status = clGetDeviceIDs(platforms[p], CL_DEVICE_TYPE_ALL, 0, NULL, &numDevices);
			if (status != CL_SUCCESS)
				LOG_CRITICAL("clGetDeviceIDs failed (2)");

			cl_device_id* devices = new cl_device_id[numDevices];
			status = clGetDeviceIDs(platforms[p], CL_DEVICE_TYPE_ALL, numDevices, devices, NULL);
			if (status != CL_SUCCESS)
				LOG_CRITICAL("clGetDeviceIDs failed (2)");

			for (int d = 0; d < (int)numDevices; d++)
			{
				cl_device_type type;
				cl_uint vendor;
				size_t sizes[3] = { 0, 0, 0 };

				clGetDeviceInfo(devices[d], CL_DEVICE_TYPE, sizeof(type), &type, NULL);
				clGetDeviceInfo(devices[d], CL_DEVICE_VENDOR_ID, sizeof(vendor), &vendor, NULL);

				// select:

				if (bestPlatform < 0)		// not yet holding anything -- we'll accept anything
				{
					bestPlatform = p;
					bestDevice = d;
					s_Platform = platforms[bestPlatform];
					s_Device = devices[bestDevice];
					bestDeviceType = type;
					bestDeviceVendor = vendor;
				}
				else					// holding something already -- can we do better?
				{
					if (bestDeviceType == CL_DEVICE_TYPE_CPU)		// holding a cpu already -- switch to a gpu if possible
					{
						if (type == CL_DEVICE_TYPE_GPU)			// found a gpu
						{										// switch to the gpu we just found
							bestPlatform = p;
							bestDevice = d;
							s_Platform = platforms[bestPlatform];
							s_Device = devices[bestDevice];
							bestDeviceType = type;
							bestDeviceVendor = vendor;
						}
					}
					else										// holding a gpu -- is a better gpu available?
					{
						if (bestDeviceVendor == ID_INTEL)			// currently holding an intel gpu
						{										// we are assuming we just found a bigger, badder nvidia or amd gpu
							bestPlatform = p;
							bestDevice = d;
							s_Platform = platforms[bestPlatform];
							s_Device = devices[bestDevice];
							bestDeviceType = type;
							bestDeviceVendor = vendor;
						}
					}
				}
			}
			delete[] devices;
		}
		delete[] platforms;


		if (bestPlatform < 0)
		{
			LOG_CRITICAL("Found no OpenCL devices!\n");
		}
		else
		{
			LOG_INFO("Best OpenCL Platform: #{}, Device: #{}", bestPlatform, bestDevice);
			LOG_INFO("Vendor: {}, Type: {}", Vendor(bestDeviceVendor), Type(bestDeviceType));
		}
	}
}
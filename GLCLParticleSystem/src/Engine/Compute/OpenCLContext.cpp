#include "glclpch.h"
#include "Engine/Compute/OpenCLContext.h"

#define ID_AMD          0x1002
#define ID_INTEL        0x8086
#define ID_NVIDIA       0x10de


namespace Engine
{
	cl_device_id OpenCLContext::s_Device = nullptr;
	cl_platform_id OpenCLContext::s_Platform = nullptr;
	cl_context OpenCLContext::s_Context = nullptr;

	void OpenCLContext::Initialize()
	{
		SelectOpenCLDevice();

		cl_int status;

		s_Context = clCreateContext(NULL, 1, &s_Device, NULL, NULL, &status);
		if (status != CL_SUCCESS)
			LOG_ERROR("clCreateContext failed");
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
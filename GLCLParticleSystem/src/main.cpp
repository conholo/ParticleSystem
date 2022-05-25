#include "glclpch.h"
#include "Engine.h"
#include <glm/glm.hpp>

#define NUM_ELEMENTS	1024*1024
#define	LOCAL_SIZE		128
#define	NUM_WORK_GROUPS	NUM_ELEMENTS/LOCAL_SIZE


int main()
{
	Engine::Log::Initialize();
	Engine::OpenCLContext::Initialize();

	float* hA = new float[NUM_ELEMENTS];
	float* hB = new float[NUM_ELEMENTS];
	float* hC = new float[NUM_ELEMENTS];

	for (int i = 0; i < NUM_ELEMENTS; i++)
		hA[i] = hB[i] = (float)sqrt((double)i);

	size_t dataSize = NUM_ELEMENTS * sizeof(float);

	Engine::Window* window = new Engine::Window("Particle Simulation", 1920, 1080);
	
	Engine::OpenCLProgram* testProgram = new Engine::OpenCLProgram("resources/cl/test.cl");
	testProgram->AddBuffer("dA", dataSize, Engine::CLBufferType::ReadOnly);
	testProgram->AddBuffer("dB", dataSize, Engine::CLBufferType::ReadOnly);
	testProgram->AddBuffer("dC", dataSize, Engine::CLBufferType::WriteOnly);
	testProgram->WriteToDeviceBufferFromHostBuffer("dA", dataSize, hA);
	testProgram->WriteToDeviceBufferFromHostBuffer("dB", dataSize, hB);

	Engine::OpenCLBuffer* dA = testProgram->GetBuffer("dA");
	Engine::OpenCLBuffer* dB = testProgram->GetBuffer("dB");
	Engine::OpenCLBuffer* dC = testProgram->GetBuffer("dC");

	testProgram->AddKernel("ArrayMultiply",
		{
			new Engine::KernelArg("dA", (void*)(dA->GetBufferID()), Engine::OpenCLBuffer::NativeSize(), Engine::KernelArgType::Global),
			new Engine::KernelArg("dB", (void*)(dB->GetBufferID()), Engine::OpenCLBuffer::NativeSize(), Engine::KernelArgType::Global),
			new Engine::KernelArg("dC", (void*)(dC->GetBufferID()), Engine::OpenCLBuffer::NativeSize(), Engine::KernelArgType::Global),
		});

	testProgram->Execute("ArrayMultiply", glm::ivec3(NUM_ELEMENTS, 1, 1), glm::ivec3(LOCAL_SIZE, 1, 1 ), 0);
	testProgram->ReadDeviceBufferToHostBuffer("dC", dataSize, hC);

	while (window->IsOpen())
	{
		Engine::RenderCommand::Clear(true, true);
		Engine::RenderCommand::ClearColor({ 0.1f, 0.1f, 0.1f, 0.1f });
		window->Update();
	}

	delete window;
}
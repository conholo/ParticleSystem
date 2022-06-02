#include "glclpch.h"

#include "Engine/Renderer/Mesh.h"
#define PI 3.14159265359

namespace Engine
{
	Mesh* MeshFactory::Create(PrimitiveType type)
	{
		switch (type)
		{
		case PrimitiveType::Box: return Box();
		case PrimitiveType::Sphere: return Sphere(1.0f);
		}

		return nullptr;
	}

	Mesh* MeshFactory::Box()
	{
		std::vector<uint32_t> indices =
		{
			// Back Face
			0, 1, 2,
			2, 3, 0,

			// Left Face
			4, 1, 0,
			0, 5, 4,

			// Right Face
			2, 6, 7,
			7, 3, 2,

			// Top Face
			0, 3, 7,
			7, 5, 0,

			// Front Face
			4, 5, 7,
			7, 6, 4,

			// Bottom Face
			4, 6, 2,
			2, 1, 4
		};

		std::vector<Vertex> vertices =
		{
			Vertex{ {-1.0f,  1.0f, -1.0f}, {0.0f, 0.0f, 0.0f} },
			Vertex{ {-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, 0.0f} },
			Vertex{ { 1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, 0.0f} },
			Vertex{ { 1.0f,  1.0f, -1.0f}, {0.0f, 0.0f, 0.0f} },

			Vertex{ {-1.0f, -1.0f,  1.0f}, {0.0f, 0.0f, 0.0f} },
			Vertex{ {-1.0f,  1.0f,  1.0f}, {0.0f, 0.0f, 0.0f} },
			Vertex{ { 1.0f, -1.0f,  1.0f}, {0.0f, 0.0f, 0.0f} },
			Vertex{ { 1.0f,  1.0f,  1.0f}, {0.0f, 0.0f, 0.0f} }
		};

		return new Mesh(vertices, indices);
	}


	Mesh* MeshFactory::Sphere(float radius)
	{
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		constexpr float latitudeBands = 30;
		constexpr float longitudeBands = 30;

		for (float latitude = 0.0f; latitude <= latitudeBands; latitude++)
		{
			const float theta = latitude * (float)PI / latitudeBands;
			const float sinTheta = glm::sin(theta);
			const float cosTheta = glm::cos(theta);

			float texT = 1.0f - theta / PI;

			for (float longitude = 0.0f; longitude <= longitudeBands; longitude++)
			{
				const float phi = longitude * 2.0f * (float)PI / longitudeBands;
				const float sinPhi = glm::sin(phi);
				const float cosPhi = glm::cos(phi);

				float texS = 1.0f - (phi / (2 * PI));

				Vertex vertex;
				vertex.Normal = { cosPhi * sinTheta, cosTheta, sinPhi * sinTheta };
				vertex.Position = { radius * vertex.Normal.x, radius * vertex.Normal.y, radius * vertex.Normal.z };
				vertices.push_back(vertex);
			}
		}

		for (uint32_t latitude = 0; latitude < (uint32_t)latitudeBands; latitude++)
		{
			for (uint32_t longitude = 0; longitude < (uint32_t)longitudeBands; longitude++)
			{
				const uint32_t first = (latitude * ((uint32_t)longitudeBands + 1)) + longitude;
				const uint32_t second = first + (uint32_t)longitudeBands + 1;

				indices.push_back(first);
				indices.push_back(first + 1);
				indices.push_back(second);

				indices.push_back(second);
				indices.push_back(first + 1);
				indices.push_back(second + 1);
			}
		}

		return new Mesh(vertices, indices);
	}
}
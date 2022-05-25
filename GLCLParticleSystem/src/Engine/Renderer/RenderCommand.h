#pragma once

#include <stdint.h>

#include <glm/glm.hpp>
#include "Engine/Renderer/VertexArray.h"

namespace Engine
{
	enum class DrawMode { None = 0, Fill, WireFrame };
	enum class RenderTopology { None = 0, Triangles, Quads };

	enum class RenderFlag
	{
		None		= 1 << 0,
		DepthTest	= 1 << 1,
		Blend		= 1 << 2,
		TwoSided	= 1 << 3,
	};

	enum class FaceCullMode { None = 0, Front, Back };

	class RenderCommand
	{
	public:
		static void Initialize();
		static void SetFaceCullMode(FaceCullMode cullMode);
		static void SetFlags(uint32_t flags);
		static void SetDrawMode(DrawMode drawMode);
		static void SetPointSize(float size);
		static void Clear(bool colorBufferBit, bool depthBufferBit);
		static void SetViewport(uint32_t width, uint32_t height);
		static void ClearColor(const glm::vec4& clearColor);
		static void DrawIndexed(VertexArray* vertexArray, uint32_t indexCount = 0, RenderTopology topology = RenderTopology::Triangles);
		static void DrawPoints(uint32_t vertexCount, uint32_t first = 0);
		static void DrawArrays(uint32_t vertexCount, uint32_t first = 0, RenderTopology topology = RenderTopology::Triangles);
	};
}


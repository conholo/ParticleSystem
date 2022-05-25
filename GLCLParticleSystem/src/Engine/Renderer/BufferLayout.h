#pragma once

#include <stdint.h>
#include <vector>
#include <string>
#include <unordered_map>

namespace Engine
{
	enum class ShaderDataType
	{
		None = 0, Float, Float2, Float3, Float4, Int, Mat3, Mat4, Sampler2D
	};

	static std::unordered_map<ShaderDataType, const char*> ShaderDataTypeToString =
	{
		{ShaderDataType::Float,		"float"},
		{ShaderDataType::Float2,	"vec2"},
		{ShaderDataType::Float3,	"vec3"},
		{ShaderDataType::Float4,	"vec4"},
		{ShaderDataType::Int,		"int"},
		{ShaderDataType::Mat3,		"mat3"},
		{ShaderDataType::Mat4,		"mat4"},
		{ShaderDataType::Sampler2D,	"sampler2D"},
	};

	static uint32_t ShaderDataTypeSize(ShaderDataType type)
	{
		switch (type)
		{
			case ShaderDataType::Float:		return 1 * 4;
			case ShaderDataType::Float2:	return 2 * 4;
			case ShaderDataType::Float3:	return 3 * 4;
			case ShaderDataType::Float4:	return 4 * 4;
			case ShaderDataType::Int:		return 1 * 4;
			case ShaderDataType::Mat3:		return 3 * 3 * 4;
			case ShaderDataType::Mat4:		return 4 * 4 * 4;
			// 3 Ints, second int represents bool for hide in inspector, made int to keep better alignment
			case ShaderDataType::Sampler2D:	return 3 * 4;
			default:						return 0;
		}
	}

	struct BufferElement
	{
		std::string Name;
		ShaderDataType Type;
		uint32_t Size;
		uint32_t Offset;
		bool Normalized;

		BufferElement() = default;

		BufferElement(const std::string& name, ShaderDataType type, bool normalized = false)
			:Name(name), Type(type), Size(ShaderDataTypeSize(type)), Offset(0), Normalized(normalized) { }

		uint32_t GetComponentCount() const
		{
			switch (Type)
			{
				case ShaderDataType::Float:		return 1;
				case ShaderDataType::Float2:	return 2;
				case ShaderDataType::Float3:	return 3;
				case ShaderDataType::Float4:	return 4;
				case ShaderDataType::Int:		return 1;
				case ShaderDataType::Mat3:		return 3 * 3;
				case ShaderDataType::Mat4:		return 4 * 4;
				default:						return 0;
			}
		}
	};


	class BufferLayout
	{
	public:
		BufferLayout() = default;

		BufferLayout(const std::initializer_list<BufferElement> elements)
			:m_Elements(elements)
		{
			CalculateStrideAndOffsets();
		}

		uint32_t GetStride() const { return m_Stride; }

		std::vector<BufferElement>& GetElements() { return m_Elements; }
		const std::vector<BufferElement>& GetElements() const { return m_Elements; }

		std::vector<BufferElement>::iterator begin() { return m_Elements.begin(); }
		std::vector<BufferElement>::iterator end() { return m_Elements.end(); }

		std::vector<BufferElement>::const_iterator begin() const { return m_Elements.begin(); }
		std::vector<BufferElement>::const_iterator end() const { return m_Elements.end(); }

	private:
		void CalculateStrideAndOffsets()
		{
			m_Stride = 0;
			uint32_t offset = 0;

			for (auto& element : m_Elements)
			{
				element.Offset = offset;
				offset += element.Size;
				m_Stride += element.Size;
			}
		}

	private:
		uint32_t m_Stride = 0;
		std::vector<BufferElement> m_Elements;
	};
}
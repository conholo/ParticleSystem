#pragma once

#include <glm/glm.hpp>

namespace Engine
{
	typedef unsigned int GLenum;
	typedef int GLint;



	class Shader
	{
	public:
		Shader(const std::string& filePath);
		~Shader();

		void Bind() const;
		void Unbind() const;

		uint32_t GetID() const { return m_ID; }
		std::string GetName() const { return m_Name; }


		GLint UploadUniformFloat(const std::string& name, float value);
		GLint UploadUniformFloat2(const std::string& name, const glm::vec2& value);
		GLint UploadUniformFloat3(const std::string& name, const glm::vec3& value);
		GLint UploadUniformFloat4(const std::string& name, const glm::vec4& value);
		GLint UploadUniformInt(const std::string& name, int value);
		GLint UploadUniformIntArray(const std::string& name, uint32_t count, int* basePtr);
		GLint UploadUniformMat3(const std::string& name, const glm::mat3& matrix);
		GLint UploadUniformMat4(const std::string& name, const glm::mat4& matrix);

		void EnableShaderImageAccessBarrierBit();
		void DispatchCompute(uint32_t groupX, uint32_t groupY, uint32_t groupZ);

	private:
		std::string ReadFile(const std::string& filePath);
		std::unordered_map<GLenum, std::string> PreProcess(const std::string& source);
		void Compile(const std::unordered_map<GLenum, std::string>& shaderSources);
		int32_t GetUniformLocation(const std::string& name);

	private:
		std::unordered_map<std::string, int32_t> m_UniformCache;
		std::string m_Name;
		uint32_t m_ID;
		bool m_IsCompute = false;
	};
}
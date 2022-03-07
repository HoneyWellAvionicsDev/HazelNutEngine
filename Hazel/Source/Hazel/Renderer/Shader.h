#pragma once

#include <string>
#include <glm/glm.hpp>

namespace Hazel
{
	class Shader
	{
	public:
		virtual ~Shader() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
		//virtual void UploadUniformMat4(const std::string& name, const glm::mat4& matrix) = 0;
		//virtual void UploadUniformFloat4(const std::string& name, const glm::vec4& vector) = 0;

		static Shader* Upload(const std::string& vertexSrc, const std::string& fragmentSrc);
	
	
	
	
	
	private:
		//uint32_t m_RendererID;
	};
}


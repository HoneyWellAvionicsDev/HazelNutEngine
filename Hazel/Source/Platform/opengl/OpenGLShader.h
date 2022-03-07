#pragma once

#include "Hazel/Renderer/Shader.h"

namespace Hazel
{
	class OpenGLShader : public Shader
	{
	public:
		OpenGLShader(const std::string& vertexSrc, const std::string& fragmentSrc);
		virtual ~OpenGLShader();

		void Bind() const override;
		void Unbind() const override;
		void UploadUniformMat4(const std::string& name, const glm::mat4& matrix) override;
		void UploadUniformFloat4(const std::string& name, const glm::vec4& vector) override;
	private:
		uint32_t m_RendererID;
	};
}


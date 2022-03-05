#pragma once

#include <string>

namespace Hazel
{
	class Shader
	{
	public:
		virtual ~Shader(){}

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		static Shader* Upload(const std::string& vertexSrc, const std::string& fragmentSrc);
	private:
		//uint32_t m_RendererID;
	};
}


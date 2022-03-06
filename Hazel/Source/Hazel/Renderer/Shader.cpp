#include "hzpch.h"
#include "Shader.h"
#include "Renderer.h"
#include "Platform/opengl/OpenGLShader.h"

#include <glad/glad.h>

namespace Hazel
{
	Shader* Shader::Upload(const std::string& vertexSrc, const std::string& fragmentSrc)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:			HZ_CORE_ASSERT(false, "You need a renderer API dumbass"); return nullptr;
		case RendererAPI::API::OpenGL:			return new OpenGLShader(vertexSrc, fragmentSrc);
		}

		HZ_CORE_ASSERT(false, "Hazel failed to detect the renderer API for unknown reasons");
		return nullptr;
	}
}

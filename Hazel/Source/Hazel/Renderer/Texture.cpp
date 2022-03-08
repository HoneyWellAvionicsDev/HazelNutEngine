#include "hzpch.h"
#include "Texture.h"
#include "Renderer.h"
#include "Platform/opengl/OpenGLTexture.h"

namespace Hazel
{
	Ref<Texture2D> Texture2D::Upload(const std::string& path)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:			HZ_CORE_ASSERT(false, "You need a renderer API dumbass"); return nullptr;
		case RendererAPI::API::OpenGL:			return std::make_shared<OpenGLTexture2D>(path);
		}

		HZ_CORE_ASSERT(false, "Hazel failed to detect the renderer API for unknown reasons");

		return nullptr;
	}

}

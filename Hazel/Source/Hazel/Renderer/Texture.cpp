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
		case RendererAPI::API::OpenGL:			return CreateScope<OpenGLTexture2D>(path);
		}

		HZ_CORE_ASSERT(false, "Hazel failed to detect the renderer API for unknown reasons");

		return nullptr;
	}

	Ref<Texture2D> Texture2D::Upload(uint32_t width, uint32_t height)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:			HZ_CORE_ASSERT(false, "You need a renderer API dumbass"); return nullptr;
		case RendererAPI::API::OpenGL:			return CreateScope<OpenGLTexture2D>(width, height);
		}

		HZ_CORE_ASSERT(false, "Hazel failed to detect the renderer API for unknown reasons");

		return nullptr;
	}
}

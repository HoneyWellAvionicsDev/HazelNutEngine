#include "hzpch.h"
#include "Texture.h"
#include "Renderer.h"
#include "Platform/opengl/OpenGLTexture.h"

namespace Jbonk
{
	Ref<Texture2D> Texture2D::Upload(const std::string& path)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:			JB_CORE_ASSERT(false, "You need a renderer API dumbass"); return nullptr;
		case RendererAPI::API::OpenGL:			return CreateScope<OpenGLTexture2D>(path);
		}

		JB_CORE_ASSERT(false, "Jbonk failed to detect the renderer API for unknown reasons");

		return nullptr;
	}

	Ref<Texture2D> Texture2D::Upload(uint32_t width, uint32_t height)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:			JB_CORE_ASSERT(false, "You need a renderer API dumbass"); return nullptr;
		case RendererAPI::API::OpenGL:			return CreateScope<OpenGLTexture2D>(width, height);
		}

		JB_CORE_ASSERT(false, "Jbonk failed to detect the renderer API for unknown reasons");

		return nullptr;
	}
}

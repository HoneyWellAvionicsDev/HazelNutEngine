#include "hzpch.h"
#include "VertexArray.h"
#include "Platform/opengl/OpenGLVertexArray.h"
#include "Renderer.h"

namespace Hazel
{
	Ref<VertexArray> VertexArray::Create()
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:			HZ_CORE_ASSERT(false, "You need a renderer API dumbass"); return nullptr;
		case RendererAPI::API::OpenGL:			return std::make_shared<OpenGLVertexArray>();
		}

		HZ_CORE_ASSERT(false, "Hazel failed to detect the renderer API for unknown reasons");
		return nullptr;
	}
}

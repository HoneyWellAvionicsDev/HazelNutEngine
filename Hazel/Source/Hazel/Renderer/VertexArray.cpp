#include "hzpch.h"
#include "VertexArray.h"
#include "Platform/opengl/OpenGLVertexArray.h"
#include "Renderer.h"

namespace Jbonk
{
	Ref<VertexArray> VertexArray::Create()
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:			JB_CORE_ASSERT(false, "You need a renderer API dumbass"); return nullptr;
			case RendererAPI::API::OpenGL:			return CreateRef<OpenGLVertexArray>();
		}

		JB_CORE_ASSERT(false, "Jbonk failed to detect the renderer API for unknown reasons");
		return nullptr;
	}
}

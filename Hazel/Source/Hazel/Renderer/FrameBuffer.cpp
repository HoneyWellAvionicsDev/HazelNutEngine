#include "hzpch.h"
#include "FrameBuffer.h"
#include "Hazel/Renderer/Renderer.h"
#include "Platform/opengl/OpenGLFrameBuffer.h"

namespace Jbonk
{
	Ref<FrameBuffer> FrameBuffer::Create(const FrameBufferSpecification& specs)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:			JB_CORE_ASSERT(false, "You need a renderer API dumbass"); return nullptr;
		case RendererAPI::API::OpenGL:			return CreateRef<OpenGLFrameBuffer>(specs);
		}

		JB_CORE_ASSERT(false, "Jbonk failed to detect the renderer API for unknown reasons");
		return nullptr;
	}

}
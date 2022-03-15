#include "hzpch.h"
#include "FrameBuffer.h"
#include "Hazel/Renderer/Renderer.h"
#include "Platform/opengl/OpenGLFrameBuffer.h"

namespace Hazel
{
	Ref<FrameBuffer> FrameBuffer::Create(const FrameBufferSpecification& specs)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:			HZ_CORE_ASSERT(false, "You need a renderer API dumbass"); return nullptr;
		case RendererAPI::API::OpenGL:			return CreateRef<OpenGLFrameBuffer>(specs);
		}

		HZ_CORE_ASSERT(false, "Hazel failed to detect the renderer API for unknown reasons");
		return nullptr;
	}

}
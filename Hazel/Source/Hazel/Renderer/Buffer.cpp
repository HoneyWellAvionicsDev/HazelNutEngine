#include <hzpch.h>

#include "Buffer.h"
#include "Renderer.h"
#include "Platform/opengl/OpenGLBuffer.h"

#if HZ_PLATFORM_WINDOWS
//#include "Platform/Direct3D/Direct3DBuffer.h"
#endif

namespace Hazel
{
	VertexBuffer* VertexBuffer::Create(float* vertices, uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::None: HZ_CORE_ASSERT(false, "You need a renderer API dumbass"); return nullptr;
		case RendererAPI::OpenGL: return new OpenGLVertexBuffer(vertices, size);
		}

		HZ_CORE_ASSERT(false, "Hazel failed to detect the renderer API for unknown reasons")
		return nullptr;
	}

	IndexBuffer* IndexBuffer::Create(uint32_t* indices, uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::None: HZ_CORE_ASSERT(false, "You need a renderer API dumbass"); return nullptr;
		case RendererAPI::OpenGL: return new OpenGLIndexBuffer(indices, size);
		}

		HZ_CORE_ASSERT(false, "Hazel failed to detect the renderer API for unknown reasons")
			return nullptr;

	
	}


}
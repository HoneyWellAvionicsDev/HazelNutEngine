#include <hzpch.h>

#include "Buffer.h"
#include "Renderer.h"
#include "Platform/opengl/OpenGLBuffer.h"

//#if HZ_PLATFORM_WINDOWS
////#include "Platform/Direct3D/Direct3DBuffer.h"
//#endif

namespace Jbonk
{
	Ref<VertexBuffer> VertexBuffer::Create(uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:			JB_CORE_ASSERT(false, "You need a renderer API dumbass"); return nullptr;
		case RendererAPI::API::OpenGL:			return CreateScope<OpenGLVertexBuffer>(size);
		}

		JB_CORE_ASSERT(false, "Jbonk failed to detect the renderer API for unknown reasons")
			return nullptr;
	}

	Ref<VertexBuffer> VertexBuffer::Create(float* vertices, uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:			JB_CORE_ASSERT(false, "You need a renderer API dumbass"); return nullptr;
		case RendererAPI::API::OpenGL:			return CreateScope<OpenGLVertexBuffer>(vertices, size);
		}

		JB_CORE_ASSERT(false, "Jbonk failed to detect the renderer API for unknown reasons")
		return nullptr;
	}

	Ref<IndexBuffer> IndexBuffer::Create(uint32_t* indices, uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:			JB_CORE_ASSERT(false, "You need a renderer API dumbass"); return nullptr;
		case RendererAPI::API::OpenGL:			return CreateScope<OpenGLIndexBuffer>(indices, size);
		}

		JB_CORE_ASSERT(false, "Jbonk failed to detect the renderer API for unknown reasons");
		return nullptr;

	
	}


}
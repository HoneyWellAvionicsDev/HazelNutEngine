#include <hzpch.h>
#include "RenderCommand.h"
#include "Platform/opengl/OpenGLRendererAPI.h"

namespace Hazel
{
	RendererAPI* RenderCommand::s_RendererAPI = new OpenGLRendererAPI;
}
#include <hzpch.h>
#include "RenderCommand.h"
#include "Platform/opengl/OpenGLRendererAPI.h"

namespace Jbonk
{
	RendererAPI* RenderCommand::s_RendererAPI = new OpenGLRendererAPI;
}
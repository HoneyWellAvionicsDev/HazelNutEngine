#pragma once

#include "RenderCommand.h"

namespace Hazel
{
	class Renderer
	{
	public:
		static void BeginScene(); //todo
		static void EndScene();

		static void Sumbit(const std::shared_ptr<VertexArray>& vertexArray);

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
	};
}


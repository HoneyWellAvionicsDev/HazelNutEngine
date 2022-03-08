#pragma once

#include "Hazel/Renderer/RendererAPI.h"

namespace Hazel
{
	class OpenGLRendererAPI : public RendererAPI
	{
		void SetClearColor(const glm::vec4& color) override;
		void Clear() override;
		void DrawIndexed(const Ref<VertexArray>& VertexArray) override;

	};
}


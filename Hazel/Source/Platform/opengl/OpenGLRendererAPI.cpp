#include "hzpch.h"
#include "OpenGLRendererAPI.h"
#include <glad/glad.h>

namespace Hazel
{
	void OpenGLRendererAPI::SetClearColor(const glm::vec4& color)
	{
		glClearColor(color.r, color.g, color.b, color.a);
	}

	void OpenGLRendererAPI::Clear()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void OpenGLRendererAPI::DrawIndexed(const std::shared_ptr<VertexArray>& VertexArray)
	{
		glDrawElements(GL_TRIANGLES, VertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr); //elements are our indices (draw mode, how many to draw, type, pointer to elements)
	}
}

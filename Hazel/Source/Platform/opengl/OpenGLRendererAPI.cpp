#include "hzpch.h"
#include "OpenGLRendererAPI.h"
#include <glad/glad.h>

namespace Hazel
{
	void OpenGLRendererAPI::Init()
	{
#if/*def*/ 0//HZ_DEBUG
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(OpenGLMessageCallback, nullptr);

		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
#endif
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_LINE_SMOOTH);
	}

	void OpenGLRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		glViewport(x, y, width, height);
	}

	void OpenGLRendererAPI::SetClearColor(const glm::vec4& color)
	{
		glClearColor(color.r, color.g, color.b, color.a);
	}

	void OpenGLRendererAPI::Clear()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void OpenGLRendererAPI::DrawIndexed(const Ref<VertexArray>& VertexArray, uint32_t indexCount)
	{
		VertexArray->Bind();
		uint32_t count = indexCount ? indexCount : VertexArray->GetIndexBuffer()->GetCount();           //maybe switch this the other way around
		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr); //elements are our indices (draw mode, how many to draw, type, pointer to elements)
	}
}

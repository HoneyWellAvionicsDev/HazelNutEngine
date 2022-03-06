#include "hzpch.h"
#include "OpenGLVertexArray.h"
#include <glad/glad.h>

namespace Hazel
{
	static GLenum ShaderDataTypeToOpenGlBaseType(ShaderDataType type)
	{
		switch (type)
		{
			case Hazel::ShaderDataType::Float:			 return GL_FLOAT;
			case Hazel::ShaderDataType::Float2:			 return GL_FLOAT;
			case Hazel::ShaderDataType::Float3:			 return GL_FLOAT;
			case Hazel::ShaderDataType::Float4:			 return GL_FLOAT;
			case Hazel::ShaderDataType::Mat3:			 return GL_FLOAT;
			case Hazel::ShaderDataType::Mat4:			 return GL_FLOAT;
			case Hazel::ShaderDataType::Int:			 return GL_INT;
			case Hazel::ShaderDataType::Int2:			 return GL_INT;
			case Hazel::ShaderDataType::Int3:			 return GL_INT;
			case Hazel::ShaderDataType::Int4:			 return GL_INT;
			case Hazel::ShaderDataType::Bool:			 return GL_BOOL;
			//case Hazel::ShaderDataType::
		}
		HZ_CORE_ASSERT(false, "Unknown ShaderDataType!")
			return 0;
	}

	OpenGLVertexArray::OpenGLVertexArray()
	{
		glCreateVertexArrays(1, &m_RendererID);
	}

	OpenGLVertexArray::~OpenGLVertexArray()
	{
		glDeleteVertexArrays(1, &m_RendererID);
	}

	void OpenGLVertexArray::Bind() const
	{
		glBindVertexArray(m_RendererID);
	}

	void OpenGLVertexArray::Unbind() const
	{
		glBindVertexArray(0);
	}

	void OpenGLVertexArray::AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer)
	{
		HZ_CORE_ASSERT(vertexBuffer->GetLayout().GetElements().size(), "Vertex buffer has no layout!");

		glBindVertexArray(m_RendererID);
		vertexBuffer->Bind();

		const auto& layout = vertexBuffer->GetLayout();
		uint32_t index = 0;
		for (const auto& elements : layout)
		{
			glEnableVertexAttribArray(index); //we need to describe our data to the GPU 
			glVertexAttribPointer(index,
				elements.GetComponentCount(),
				ShaderDataTypeToOpenGlBaseType(elements.Type),
				elements.Normalized ? GL_TRUE : GL_FALSE,
				layout.GetStride(),
				(const void*)elements.Offset); //(index, #of vars, data type, normalized?, size of data, pointer)
			index++;
		}
		m_VertexBuffers.push_back(vertexBuffer);
	}


	void OpenGLVertexArray::SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer)
	{
		glBindVertexArray(m_RendererID);
		indexBuffer->Bind();

		m_IndexBuffer = indexBuffer;
	}
}

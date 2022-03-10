#include "hzpch.h"
#include "Renderer2D.h"
#include "VertexArray.h"
#include "Shader.h"
#include "RenderCommand.h"
#include <glm/gtc/matrix_transform.hpp>



namespace Hazel
{
	struct Renderer2Ddata
	{
		Ref<VertexArray> QuadVertexArray;
		Ref<Shader> m_FlatColorShader;
	};

	static Renderer2Ddata* s_Data;

	void Renderer2D::Init()
	{
		s_Data = new Renderer2Ddata();

		s_Data->QuadVertexArray = VertexArray::Create();

		float sqVertices[3 * 4] =
		{
			-0.5f, -0.5f, 0.f,
			 0.5f, -0.5f, 0.f,
			 0.5f,  0.5f, 0.f,
			-0.5f,  0.5f, 0.f
		};
		Ref<VertexBuffer> squareVB;
		squareVB.reset(VertexBuffer::Create(sqVertices, sizeof(sqVertices)));


		BufferLayout sqLayout =
		{
			{ShaderDataType::Float3, "a_Posistion" }
		};
		squareVB->SetLayout(sqLayout);
		s_Data->QuadVertexArray->AddVertexBuffer(squareVB);

		uint32_t sqIndices[6] = { 0,1,2,2,3,0 };
		Ref<IndexBuffer> squareIB;
		squareIB.reset(IndexBuffer::Create(sqIndices, sizeof(sqIndices) / sizeof(uint32_t)));
		s_Data->QuadVertexArray->SetIndexBuffer(squareIB);



		s_Data->m_FlatColorShader = Shader::Upload("assets/shaders/FlatColorShader.glsl");
	}

	void Renderer2D::Shutdown()
	{
		delete s_Data;
	}

	void Renderer2D::BeginScene(const OrthographicCamera& camera)
	{
		s_Data->m_FlatColorShader->Bind();
		s_Data->m_FlatColorShader->SetMat4("u_ViewProjection", camera.GetViewProjectionMatrix());
	}

	void Renderer2D::EndScene()
	{
	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
	{
		DrawQuad({ position.x, position.y, 0.f }, size, color);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
	{
		s_Data->m_FlatColorShader->Bind();
		s_Data->m_FlatColorShader->SetFloat4("u_Color", color);
		glm::mat4 transform = glm::translate(glm::mat4(1.f), position) * glm::scale(glm::mat4(1.f), { size.x, size.y, 1.f });
		s_Data->m_FlatColorShader->SetMat4("u_Transform", transform);

		s_Data->QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(s_Data->QuadVertexArray);
		
	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture)
	{
		DrawQuad({ position.x, position.y, 0.f }, size, texture);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture)
	{
		s_Data->m_FlatColorShader->Bind();
		s_Data->m_FlatColorShader->SetFloat4("u_Color", color);
		glm::mat4 transform = glm::translate(glm::mat4(1.f), position) * glm::scale(glm::mat4(1.f), { size.x, size.y, 1.f });
		s_Data->m_FlatColorShader->SetMat4("u_Transform", transform);

		s_Data->QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(s_Data->QuadVertexArray);
	}
}

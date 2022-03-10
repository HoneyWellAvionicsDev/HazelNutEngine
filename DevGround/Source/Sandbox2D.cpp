#include "Sandbox2D.h"
#include <imgui.h>
//#include <glm/gtc/matrix_transform.hpp>
#include "glm/gtc/type_ptr.hpp"
//temp
#include "Platform/opengl/OpenGLShader.h"

Sandbox2D::Sandbox2D()
	: Layer("Sandbox2D"), m_CameraController(1280.f / 720.f, true)
{
}

void Sandbox2D::OnAttach()
{
	
	//-------------------------------------------------------------------------------------------------
	m_SquareVA = Hazel::VertexArray::Create();

	float sqVertices[3 * 4] =             
	{
		-0.5f, -0.5f, 0.f,
		 0.5f, -0.5f, 0.f,
		 0.5f,  0.5f, 0.f,
		-0.5f,  0.5f, 0.f
	};
	Hazel::Ref<Hazel::VertexBuffer> squareVB;
	squareVB.reset(Hazel::VertexBuffer::Create(sqVertices, sizeof(sqVertices)));


	Hazel::BufferLayout sqLayout =
	{
		{Hazel::ShaderDataType::Float3, "a_Posistion" }
	};
	squareVB->SetLayout(sqLayout);
	m_SquareVA->AddVertexBuffer(squareVB);

	uint32_t sqIndices[6] = { 0,1,2,2,3,0 };
	Hazel::Ref<Hazel::IndexBuffer> squareIB;
	squareIB.reset(Hazel::IndexBuffer::Create(sqIndices, sizeof(sqIndices) / sizeof(uint32_t)));
	m_SquareVA->SetIndexBuffer(squareIB);
	//-------------------------------------------------------------------------------------------------
	

	m_FlatColorShader = Hazel::Shader::Upload("assets/shaders/FlatColorShader.glsl");
}

void Sandbox2D::OnDetach()
{

}

void Sandbox2D::OnUpdate(Hazel::Timestep ts)
{
	//------------------Update----------------------------------
	m_CameraController.OnUpdate(ts);

	//------------------Render----------------------------------
	Hazel::RenderCommand::SetClearColor({ 0.04f, 0.04f, 0.04f, 1 });
	Hazel::RenderCommand::Clear();

	Hazel::Renderer::BeginScene(m_CameraController.GetCamera());

	


	std::dynamic_pointer_cast<Hazel::OpenGLShader>(m_FlatColorShader)->Bind();
	std::dynamic_pointer_cast<Hazel::OpenGLShader>(m_FlatColorShader)->UploadUniformFloat4("u_Color", m_SquareColor);

	Hazel::Renderer::Sumbit(m_SquareVA, m_FlatColorShader, glm::scale(glm::mat4(1.f), glm::vec3(1.5f)));


	Hazel::Renderer::EndScene();
}

void Sandbox2D::OnImGuiRender()
{
	ImGui::Begin("Settings");
	ImGui::ColorEdit4("Square Color", glm::value_ptr(m_SquareColor));
	ImGui::End();
}

void Sandbox2D::OnEvent(Hazel::Event& event)
{
	m_CameraController.OnEvent(event);
}

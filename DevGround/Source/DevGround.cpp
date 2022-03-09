#include <Hazel.h>
#include <imgui.h>

#include <glm/gtc/matrix_transform.hpp>
#include "Platform/opengl/OpenGLShader.h"
#include "glm/gtc/type_ptr.hpp"

class ExampleLayer : public Hazel::Layer
{
public:
	ExampleLayer()
		: Layer("Test"), m_CameraController(1280.f / 720.f, true)
	{
		m_VertexArray.reset(Hazel::VertexArray::Create());               //creates the vertex array

		float vertices[3 * 7] = //currently this data exsists in the CPU
		{
			-0.9f, -0.7f, 0.f, 0.54f, 0.21f, 0.36f, 1.f,
			 0.9f, -0.7f, 0.f, 0.34f, 0.54f, 0.66f, 1.f,
			 0.f,   0.8f, 0.f, 0.98f, 0.76f, 0.06f, 1.f
		};
		//so lets move it over to the GPU
		Hazel::Ref<Hazel::VertexBuffer> vertexBuffer;
		vertexBuffer.reset(Hazel::VertexBuffer::Create(vertices, sizeof(vertices)));

		Hazel::BufferLayout layout =
		{
			{Hazel::ShaderDataType::Float3, "a_Posistion" },
			{Hazel::ShaderDataType::Float4, "a_Color" }
		};
		vertexBuffer->SetLayout(layout);
		m_VertexArray->AddVertexBuffer(vertexBuffer);       //binds vertex buffer to vertex array

		uint32_t indices[3] = { 0,1,2 };
		Hazel::Ref<Hazel::IndexBuffer> indexBuffer;
		indexBuffer.reset(Hazel::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
		m_VertexArray->SetIndexBuffer(indexBuffer);
		//-------------------------------------------------------------------------------------------------
		m_SquareVA.reset(Hazel::VertexArray::Create());

		float sqVertices[5 * 4] =             //if 0.5f has - then its 0.f, positive then its 1.f
		{
			-0.5f, -0.5f, 0.f, 0.f, 0.f,
			 0.5f, -0.5f, 0.f, 1.f, 0.f,
			 0.5f,  0.5f, 0.f, 1.f, 1.f,
			-0.5f,  0.5f, 0.f, 0.f, 1.f  
		};
		Hazel::Ref<Hazel::VertexBuffer> squareVB;
		squareVB.reset(Hazel::VertexBuffer::Create(sqVertices, sizeof(sqVertices)));


		Hazel::BufferLayout sqLayout =
		{
			{Hazel::ShaderDataType::Float3, "a_Posistion" },
			{Hazel::ShaderDataType::Float2, "a_TextureCoord" }
		};
		squareVB->SetLayout(sqLayout);
		m_SquareVA->AddVertexBuffer(squareVB);

		uint32_t sqIndices[6] = { 0,1,2,2,3,0 };
		Hazel::Ref<Hazel::IndexBuffer> squareIB;
		squareIB.reset(Hazel::IndexBuffer::Create(sqIndices, sizeof(sqIndices) / sizeof(uint32_t)));
		m_SquareVA->SetIndexBuffer(squareIB);
		//-------------------------------------------------------------------------------------------------
		std::string vertexSrc = R"(
			#version 330 core
			
			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec4 a_Color;

			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;

			out vec3 v_Position;
			out vec4 v_Color;

			void main()
			{
				v_Position = a_Position;
				v_Color = a_Color;
				gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
			}
		)";

		std::string fragmentSrc = R"(
			#version 330 core
			
			layout(location = 0) out vec4 color;

			in vec3 v_Position;
			in vec4 v_Color;

			void main()
			{
				color = vec4(v_Position * 0.5 + 0.5, 0.34);
				color = v_Color;
			}
		)";
		m_Shader = Hazel::Shader::Upload("VertexColorTri", vertexSrc, fragmentSrc);

		

		std::string vertexSrc2 = R"(
			#version 330 core
			
			layout(location = 0) in vec3 a_Position;

			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;

			out vec3 v_Position;
			out vec4 v_Color;

			void main()
			{
				v_Position = a_Position;
				gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
			}
		)";

		std::string fragmentSrc2 = R"(
			#version 330 core
			
			layout(location = 0) out vec4 color;

			in vec3 v_Position;
			in vec4 v_Color;

			uniform vec3 u_Color;

			void main()
			{
				color = vec4(u_Color, 1.0);
			}
		)";

		m_Shader2 = Hazel::Shader::Upload("FlatColor", vertexSrc2, fragmentSrc2);

		m_Texture = Hazel::Texture2D::Upload("assets/textures/Space.png");
		auto textureShader = m_ShaderLibrary.Load("assets/shaders/Texture.glsl");

		std::dynamic_pointer_cast<Hazel::OpenGLShader>(textureShader)->Bind();
		std::dynamic_pointer_cast<Hazel::OpenGLShader>(textureShader)->UploadUniformInt("u_Texture", 0);
	}

	void OnUpdate(Hazel::Timestep ts) override
	{
		//------------------Update----------------------------------
		m_CameraController.OnUpdate(ts);

		//------------------Render----------------------------------
		Hazel::RenderCommand::SetClearColor({ 0.04f, 0.04f, 0.04f, 1 });
		Hazel::RenderCommand::Clear();

		Hazel::Renderer::BeginScene(m_CameraController.GetCamera());

		//Hazel::MaterialRef material = new Hazel::Material(m_Shader2); this is an end goal for our API
		//material->Set("u_Color", redColor);
		//squareMesh->SetMaterial(material);
		
		glm::mat4 scale = glm::scale(glm::mat4(1.f), glm::vec3(0.1f));
		std::dynamic_pointer_cast<Hazel::OpenGLShader>(m_Shader2)->Bind();
		std::dynamic_pointer_cast<Hazel::OpenGLShader>(m_Shader2)->UploadUniformFloat3("u_Color", m_SquareColor);

		for (int y = 0; y < 20; y++)
		{
			for (int x = 0; x < 20; x++)
			{
				glm::vec3 pos(x * 0.11f, y * 0.11f, 0.f);
				glm::mat4 transform = glm::translate(glm::mat4(1.f), pos) * scale;
				Hazel::Renderer::Sumbit(m_SquareVA, m_Shader2, transform);
			}
		}

		auto textureShader = m_ShaderLibrary.Get("Texture");

		m_Texture->Bind();
		Hazel::Renderer::Sumbit(m_SquareVA, textureShader, glm::scale(glm::mat4(1.f), glm::vec3(1.5f)));

		//Hazel::Renderer::Sumbit(m_VertexArray, m_Shader); triangle

		Hazel::Renderer::EndScene();
	}

	void OnImGuiRender() override
	{
		ImGui::Begin("Settings");
		ImGui::ColorEdit3("Square Color", glm::value_ptr(m_SquareColor));


		ImGui::End();
	}

	void OnEvent(Hazel::Event& event) override
	{
		m_CameraController.OnEvent(event);
	}

	bool OnKeyPressedEvent(Hazel::KeyPressedEvent& event)
	{
		return false;
	}
private:
	Hazel::ShaderLibrary m_ShaderLibrary;
	Hazel::Ref<Hazel::Shader> m_Shader;
	Hazel::Ref<Hazel::VertexArray> m_VertexArray;                   
	Hazel::Ref<Hazel::Shader> m_Shader2;
	Hazel::Ref<Hazel::VertexArray> m_SquareVA;
	Hazel::Ref<Hazel::Texture2D> m_Texture;

	Hazel::OrthographicCameraController m_CameraController;
	
	glm::vec3 m_SquareColor = { 0.2f, 0.3f, 0.8f };
};

class Sandbox : public Hazel::Application
{
public:
	Sandbox()
	{
		PushLayer(new ExampleLayer());
	}
	~Sandbox()
	{

	}
};

Hazel::Application* Hazel::CreateApplication()
{
	return new Sandbox();
}

























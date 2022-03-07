#include <Hazel.h>
#include <imgui.h>

#include <glm/gtc/matrix_transform.hpp>

class ExampleLayer : public Hazel::Layer
{
public:
	ExampleLayer()
		: Layer("Test"), m_Camera(-1.6f, 1.6f, -0.9f, 0.9f), m_CameraPosition(0.f), m_SquarePosition(0.f)
	{
		m_VertexArray.reset(Hazel::VertexArray::Create());               //creates the vertex array

		float vertices[3 * 7] = //currently this data exsists in the CPU
		{
			-0.9f, -0.7f, 0.f, 0.54f, 0.21f, 0.36f, 1.f,
			 0.9f, -0.7f, 0.f, 0.34f, 0.54f, 0.66f, 1.f,
			 0.f,   0.8f, 0.f, 0.98f, 0.76f, 0.06f, 1.f
		};
		//so lets move it over to the GPU
		std::shared_ptr<Hazel::VertexBuffer> vertexBuffer;
		vertexBuffer.reset(Hazel::VertexBuffer::Create(vertices, sizeof(vertices)));

		Hazel::BufferLayout layout =
		{
			{Hazel::ShaderDataType::Float3, "a_Posistion" },
			{Hazel::ShaderDataType::Float4, "a_Color" },
		};
		vertexBuffer->SetLayout(layout);
		m_VertexArray->AddVertexBuffer(vertexBuffer);       //binds vertex buffer to vertex array

		uint32_t indices[3] = { 0,1,2 };
		std::shared_ptr<Hazel::IndexBuffer> indexBuffer;
		indexBuffer.reset(Hazel::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
		m_VertexArray->SetIndexBuffer(indexBuffer);
		//-------------------------------------------------------------------------------------------------
		m_SquareVA.reset(Hazel::VertexArray::Create());

		float sqVertices[3 * 4] =
		{
			-0.5f, -0.5f, 0.f,
			 0.5f, -0.5f, 0.f,
			 0.5f,  0.5f, 0.f,
			-0.5f,  0.5f, 0.f
		};
		std::shared_ptr<Hazel::VertexBuffer> squareVB;
		squareVB.reset(Hazel::VertexBuffer::Create(sqVertices, sizeof(sqVertices)));


		Hazel::BufferLayout sqLayout =
		{
			{Hazel::ShaderDataType::Float3, "a_Posistion" }
		};
		squareVB->SetLayout(sqLayout);
		m_SquareVA->AddVertexBuffer(squareVB);

		uint32_t sqIndices[6] = { 0,1,2,2,3,0 };
		std::shared_ptr<Hazel::IndexBuffer> squareIB;
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
		m_Shader.reset(Hazel::Shader::Upload(vertexSrc, fragmentSrc));

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

			uniform vec4 u_Color;

			void main()
			{
				color = u_Color;
			}
		)";

		m_Shader2.reset(Hazel::Shader::Upload(vertexSrc2, fragmentSrc2));
	}

	void OnUpdate(Hazel::Timestep ts) override
	{
		Hazel::RenderCommand::SetClearColor({ 0.04f, 0.04f, 0.04f, 1 });
		Hazel::RenderCommand::Clear();

		m_Camera.SetPosition(m_CameraPosition);
		m_Camera.SetRotation(m_CameraRotationAngle);

		if (Hazel::Input::IsKeyPressed(HZ_KEY_W))
			m_CameraPosition.y += m_CameraMoveSpeed * ts;
		else if (Hazel::Input::IsKeyPressed(HZ_KEY_S))
			m_CameraPosition.y -= m_CameraMoveSpeed * ts;
		if (Hazel::Input::IsKeyPressed(HZ_KEY_A))
			m_CameraPosition.x -= m_CameraMoveSpeed * ts;
		if (Hazel::Input::IsKeyPressed(HZ_KEY_D))
			m_CameraPosition.x += m_CameraMoveSpeed * ts;
		if (Hazel::Input::IsKeyPressed(HZ_KEY_Q))
			m_CameraRotationAngle -= m_CameraRotationSpeed * ts;
		if (Hazel::Input::IsKeyPressed(HZ_KEY_E))
			m_CameraRotationAngle += m_CameraRotationSpeed * ts;

		Hazel::Renderer::BeginScene(m_Camera);

		glm::mat4 scale = glm::scale(glm::mat4(1.f), glm::vec3(0.1f));
		glm::vec4 redColor(0.8f, 0.3f, 0.2f, 1.0f);
		glm::vec4 blueColor(0.2f, 0.3f, 0.8f, 1.0f);
		//Hazel::MaterialRef material = new Hazel::Material(m_Shader2); this is an end goal for our API
		//material->Set("u_Color", redColor);
		//squareMesh->SetMaterial(material);
		for (int y = 0; y < 20; y++)
		{
			for (int x = 0; x < 20; x++)
			{
				glm::vec3 pos(x * 0.11f, y * 0.11f, 0.f);
				glm::mat4 transform = glm::translate(glm::mat4(1.f), pos) * scale;
				if (x % 2 == 0)
					m_Shader2->UploadUniformFloat4("u_Color", redColor);
				else
					m_Shader2->UploadUniformFloat4("u_Color", blueColor);
				Hazel::Renderer::Sumbit(m_SquareVA, m_Shader2, transform);
			}
		}

		Hazel::Renderer::Sumbit(m_VertexArray, m_Shader);

		Hazel::Renderer::EndScene();
	}

	void OnImGuiRender() override
	{
		
	}

	void OnEvent(Hazel::Event& event) override
	{
		Hazel::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<Hazel::KeyPressedEvent>(HZ_BIND_EVENT_FN(ExampleLayer::OnKeyPressedEvent));
	}

	bool OnKeyPressedEvent(Hazel::KeyPressedEvent& event)
	{
		return false;
	}
private:
	std::shared_ptr<Hazel::Shader> m_Shader;
	std::shared_ptr<Hazel::VertexArray> m_VertexArray;                    //in the future we will create a new name for shared ptr
	std::shared_ptr<Hazel::Shader> m_Shader2;
	std::shared_ptr<Hazel::VertexArray> m_SquareVA;

	Hazel::OrthographicCamera m_Camera;
	glm::vec3 m_CameraPosition;
	float m_CameraMoveSpeed = 1.5f;
	float m_CameraRotationSpeed = 45.f;
	float m_CameraRotationAngle = 0.f;

	glm::vec3 m_SquarePosition;
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

























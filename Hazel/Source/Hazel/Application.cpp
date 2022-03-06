#include "hzpch.h"
#include "Application.h"

#include "Hazel/Log.h"
#include "Hazel/Input.h"
#include "Hazel/Renderer/Renderer.h"

namespace Hazel
{
#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)

	Application* Application::s_Instance = nullptr;

	Application::Application()
		: m_Camera(-1.6f, 1.6f, -0.9f, 0.9f)
	{
		HZ_CORE_ASSERT(!s_Instance, "Application already exsists!");
		s_Instance = this;

		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(BIND_EVENT_FN(OnEvent));

		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);
		//-------------------------------------------------------------------------------------------------
		m_VertexArray.reset(VertexArray::Create());               //creates the vertex array

		float vertices[3 * 7] = //currently this data exsists in the CPU
		{
			-0.9f, -0.7f, 0.f, 0.54f, 0.21f, 0.36f, 1.f,
			 0.9f, -0.7f, 0.f, 0.34f, 0.54f, 0.66f, 1.f,
			 0.f,   0.8f, 0.f, 0.98f, 0.76f, 0.06f, 1.f
		};
		//so lets move it over to the GPU
		std::shared_ptr<VertexBuffer> vertexBuffer;
		vertexBuffer.reset(VertexBuffer::Create(vertices, sizeof(vertices)));
		
		BufferLayout layout =
		{
			{ShaderDataType::Float3, "a_Posistion" },
			{ShaderDataType::Float4, "a_Color" },
		};
		vertexBuffer->SetLayout(layout);
		m_VertexArray->AddVertexBuffer(vertexBuffer);       //binds vertex buffer to vertex array

		uint32_t indices[3] = { 0,1,2 };
		std::shared_ptr<IndexBuffer> indexBuffer;
		indexBuffer.reset(IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
		m_VertexArray->SetIndexBuffer(indexBuffer);
		//-------------------------------------------------------------------------------------------------
		m_SquareVA.reset(VertexArray::Create());

		float sqVertices[3 * 4] = 
		{
			-3.f, -3.f, 0.f, 
			 3.f, -3.f, 0.f, 
			 3.f,  3.f, 0.f,
			-3.f,  3.f, 0.f
		};
		std::shared_ptr<VertexBuffer> squareVB; 
		squareVB.reset(VertexBuffer::Create(sqVertices, sizeof(sqVertices)));


		BufferLayout sqLayout =
		{
			{ShaderDataType::Float3, "a_Posistion" }
		};
		squareVB->SetLayout(sqLayout);
		m_SquareVA->AddVertexBuffer(squareVB);

		uint32_t sqIndices[6] = { 0,1,2,2,3,0 };
		std::shared_ptr<IndexBuffer> squareIB;
		squareIB.reset(IndexBuffer::Create(sqIndices, sizeof(sqIndices) / sizeof(uint32_t)));
		m_SquareVA->SetIndexBuffer(squareIB);
		//-------------------------------------------------------------------------------------------------
		std::string vertexSrc = R"(
			#version 330 core
			
			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec4 a_Color;

			uniform mat4 u_ViewProjection;

			out vec3 v_Position;
			out vec4 v_Color;

			void main()
			{
				v_Position = a_Position;
				v_Color = a_Color;
				gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
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
		m_Shader.reset(Shader::Upload(vertexSrc, fragmentSrc));

		std::string vertexSrc2 = R"(
			#version 330 core
			
			layout(location = 0) in vec3 a_Position;

			uniform mat4 u_ViewProjection;

			out vec3 v_Position;
			out vec4 v_Color;

			void main()
			{
				v_Position = a_Position;
				gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
			}
		)";

		std::string fragmentSrc2 = R"(
			#version 330 core
			
			layout(location = 0) out vec4 color;

			in vec3 v_Position;
			in vec4 v_Color;

			void main()
			{
				color = vec4(v_Position * 0.9 + 0.1, 0.34);
			}
		)";

		m_Shader2.reset(Shader::Upload(vertexSrc2, fragmentSrc2));
	}

	Application::~Application()
	{

	}

	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* layer)
	{
		m_LayerStack.PushOverlay(layer);
		layer->OnAttach();
	}

	void Application::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClose));                 //if dispatcher recieves windowCloseEvent then we call OnWindowClose
		//HZ_CORE_TRACE("{0}", event);

		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();)                      //collects all events from the layer stack from TOP to DOWN
		{
			(*--it)->OnEvent(event);
			if (event.Handled)
				break;
		}
	}

	void Application::Run()
	{
		
		while (m_Running)
		{
			for (Layer* layer : m_LayerStack)                                               //updates all the layers in the layerstack from BOTTOM to TOP
				layer->OnUpdate();

			m_ImGuiLayer->Begin();
			for (Layer* layer : m_LayerStack)
				layer->OnImGuiRender();
			m_ImGuiLayer->End();

			m_Window->OnUpdate();
			//--------------------------------------------------------------------------------------------------
			RenderCommand::SetClearColor({ 0.04f, 0.04f, 0.04f, 1 });
			RenderCommand::Clear();

			glm::vec3 lastPos = m_Camera.GetPosition();
			float rotationAngle = m_Camera.GetRotation();
			if (Hazel::Input::IsKeyPressed(HZ_KEY_W))
				m_Camera.SetPosition({lastPos.x, lastPos.y + 0.02f, lastPos.z});
			if (Hazel::Input::IsKeyPressed(HZ_KEY_S))
				m_Camera.SetPosition({ lastPos.x, lastPos.y - 0.02f, lastPos.z });
			if (Hazel::Input::IsKeyPressed(HZ_KEY_A))
				m_Camera.SetPosition({ lastPos.x - 0.02f, lastPos.y, lastPos.z });
			if (Hazel::Input::IsKeyPressed(HZ_KEY_D))
				m_Camera.SetPosition({ lastPos.x + 0.02f, lastPos.y, lastPos.z });
			if (Hazel::Input::IsKeyPressed(HZ_KEY_Q))
				m_Camera.SetRotation(rotationAngle - 1);
			if (Hazel::Input::IsKeyPressed(HZ_KEY_E))
				m_Camera.SetRotation(rotationAngle + 1);


			Renderer::BeginScene(m_Camera);
			
			Renderer::Sumbit(m_SquareVA, m_Shader2);
			Renderer::Sumbit(m_VertexArray, m_Shader);

			Renderer::EndScene();

			//--------------------------------------------------------------------------------------------------
		}
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false; //terminates the loop and exits appilcation
		return true;
	}
}
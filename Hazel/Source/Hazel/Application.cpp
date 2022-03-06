#include "hzpch.h"

#include "Application.h"

#include "Hazel/Log.h"
//#include <GLFW/glfw3.h>
//#include <glad/glad.h>
#include "Hazel/Renderer/Renderer.h"

#include "Hazel/Input.h"


namespace Hazel
{
#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)

	Application* Application::s_Instance = nullptr;

	Application::Application()
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
			-0.5f, -0.5f, 0.f, 
			 0.5f, -0.5f, 0.f, 
			 0.5f,  0.5f, 0.f,
			-0.5f,  0.5f, 0.f
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

			out vec3 v_Position;
			out vec4 v_Color;

			void main()
			{
				v_Position = a_Position;
				v_Color = a_Color;
				gl_Position = vec4(a_Position, 1.0);
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

			out vec3 v_Position;
			out vec4 v_Color;

			void main()
			{
				v_Position = a_Position;
				gl_Position = vec4(a_Position, 1.0);
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

	void Application::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClose));                 //if dispatcher recieves windowCloseEvent then we call OnWindowClose
		//HZ_CORE_TRACE("{0}", e);

		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();)                      //collects all events from the layer stack from TOP to DOWN
		{
			(*--it)->OnEvent(e);
			if (e.Handled)
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
			Renderer::BeginScene();

			m_Shader2->Bind();
			Renderer::Sumbit(m_SquareVA);
			m_Shader->Bind();
			Renderer::Sumbit(m_VertexArray);

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

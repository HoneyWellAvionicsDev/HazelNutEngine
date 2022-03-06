#include "hzpch.h"

#include "Application.h"

#include "Hazel/Log.h"
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include "Hazel/Input.h"


namespace Hazel
{
#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)

	Application* Application::s_Instance = nullptr;

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

	Application::Application()
	{
		HZ_CORE_ASSERT(!s_Instance, "Application already exsists!");
		s_Instance = this;

		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(BIND_EVENT_FN(OnEvent));

		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);

		glGenVertexArrays(1, &m_VertexArray);
		glBindVertexArray(m_VertexArray);

		

		float vertices[3 * 7] = //currently this data exsists in the CPU
		{
			-0.9f, -0.7f, 0.f, 0.54f, 0.21f, 0.36f, 1.f,
			 0.9f, -0.7f, 0.f, 0.34f, 0.54f, 0.66f, 1.f,
			 0.f,   0.8f, 0.f, 0.98f, 0.76f, 0.06f, 1.f
		};
		//so lets move it over to the GPU

		m_VertexBuffer.reset(VertexBuffer::Create(vertices, sizeof(vertices)));

		{
			BufferLayout layout =
			{
				{ShaderDataType::Float3, "a_Posistion" },
				{ShaderDataType::Float4, "a_Color" },
			};
			m_VertexBuffer->SetLayout(layout);
		}

		const auto& layout = m_VertexBuffer->GetLayout();
		
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


		uint32_t indices[3] = { 0,1,2 };
		m_IndexBuffer.reset(IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
		

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

			glClearColor(0.04f, 0.04f, 0.04f, 1);
			glClear(GL_COLOR_BUFFER_BIT);

			m_Shader->Bind();
			glBindVertexArray(m_VertexArray);
			glDrawElements(GL_TRIANGLES, m_IndexBuffer->GetCount(), GL_UNSIGNED_INT, nullptr);  //elements are our indices (draw mode, how many to draw, type, pointer to elements)
		}
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false; //terminates the loop and exits appilcation
		return true;
	}
}

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

	Application::Application()
	{
		HZ_CORE_ASSERT(!s_Instance, "Application already exsists!");
		s_Instance = this;

		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(BIND_EVENT_FN(OnEvent));
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

			auto [x, y] = Input::GetMousePosition();
			HZ_CORE_TRACE("{0}, {1}", x, y);

			m_Window->OnUpdate();

			glClearColor(0.24f, 0, 1, 1);
			glClear(GL_COLOR_BUFFER_BIT);
		}
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false; //terminates the loop and exits appilcation
		return true;
	}
}

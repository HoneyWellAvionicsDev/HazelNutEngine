#pragma once

#include "Core.h"

#include "Window.h"
#include "Hazel/LayerStack.h"
#include "Hazel/Events/Event.h"
#include "Hazel/Events/ApplicationEvent.h"
#include "Hazel/imGui/imGuiLayer.h"
#include "Hazel/Core/TimeStep.h"

namespace Hazel
{
	class HAZEL_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		inline static Application& Get() { return *s_Instance; }
		inline Window& GetWindow() { return *m_Window; }
	private:
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);
		bool m_Running = true;
		bool m_Minimized = false;

		Scope<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		LayerStack m_LayerStack;
		float m_LastFrameTime = 0.f;

		static Application* s_Instance;

	};

	Application* CreateApplication(); //to be defined in client
}



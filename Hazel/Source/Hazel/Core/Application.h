#pragma once

#include "Core.h"

#include "Window.h"
#include "Hazel/Core/LayerStack.h"
#include "Hazel/Events/Event.h"
#include "Hazel/Events/ApplicationEvent.h"
#include "Hazel/imGui/imGuiLayer.h"
#include "Hazel/Core/TimeStep.h"

namespace Hazel
{
	struct ApplicationCommandLineArgs
	{
		int Count = 0;
		char** Args = nullptr;

		const char* operator[](int index) const
		{
			HZ_CORE_ASSERT(index < Count, "Out of bounds");
			return Args[index];
		}
	};

	class Application
	{
	public:
		Application(ApplicationCommandLineArgs args, const std::string& name = "Hazel Application");
		virtual ~Application();

		void Run();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);
		void CloseWindow() { m_Running = false; }
		ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }
		Window& GetWindow() { return *m_Window; }
		ApplicationCommandLineArgs GetCommandLineArgs() const { return m_CommandLineArgs; }

		static Application& Get() { return *s_Instance; }
	private:
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);
	private:
		bool m_Running = true;
		bool m_Minimized = false;

		ApplicationCommandLineArgs m_CommandLineArgs;
		Scope<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		LayerStack m_LayerStack;
		float m_LastFrameTime = 0.f;

		static Application* s_Instance;
	};

	Application* CreateApplication(ApplicationCommandLineArgs args); //to be defined in client
}



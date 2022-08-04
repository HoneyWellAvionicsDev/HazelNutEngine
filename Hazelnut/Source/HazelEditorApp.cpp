#include <Hazel.h>
#include <Hazel/Core/EntryPoint.h>

#include "HazelEditorLayer.h"

namespace Hazel
{
	class HazelEditor : public Application
	{
	public:
		HazelEditor(const ApplicationSpecification& specification)
			: Application(specification)
		{
			PushLayer(new EditorLayer());
		}
	
		~HazelEditor()
		{
	
		}
	};
	
	Application* CreateApplication(ApplicationCommandLineArgs args)
	{
		ApplicationSpecification spec;
		spec.Name = "Hazelnut";
		spec.CommandLineArgs = args;

		return new HazelEditor(spec);
	}
}
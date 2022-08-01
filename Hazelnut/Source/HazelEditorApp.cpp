#include <Hazel.h>
#include <Hazel/Core/EntryPoint.h>

#include "HazelEditorLayer.h"

namespace Hazel
{
	class HazelEditor : public Application
	{
	public:
		HazelEditor(ApplicationCommandLineArgs args)
			: Application(args ,"Hazel Editor")
		{
			PushLayer(new EditorLayer());
		}
	
		~HazelEditor()
		{
	
		}
	};
	
	Application* CreateApplication(ApplicationCommandLineArgs args)
	{
		return new HazelEditor(args);
	}
}
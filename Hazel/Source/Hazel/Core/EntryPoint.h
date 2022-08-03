#pragma once

#include "Hazel/Core/Core.h"
#include "Hazel/Core/Application.h"

#ifdef HZ_PLATFORM_WINDOWS

extern Hazel::Application* Hazel::CreateApplication(ApplicationCommandLineArgs args);      //extern that will be found in DevGround.cpp
	
int main(int argc, char** argv)
{
	Hazel::Log::Init();

	HZ_PROFILE_BEGIN_SESSION("Startup", "HazelProfile-Startup.json");
	auto app = Hazel::CreateApplication({ argc, argv });                 //creates instance of applicaiton
	HZ_PROFILE_END_SESSION();

	HZ_PROFILE_BEGIN_SESSION("Runtime", "HazelProfile-Runtime.json");
	app->Run();
	HZ_PROFILE_END_SESSION();

	HZ_PROFILE_BEGIN_SESSION("Shutdown", "HazelProfile-Shutdown.json");
	delete app;
	HZ_PROFILE_END_SESSION();
}

#endif


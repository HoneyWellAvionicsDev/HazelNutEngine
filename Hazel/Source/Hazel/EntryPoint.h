#pragma once

#ifdef HZ_PLATFORM_WINDOWS

extern Hazel::Application* Hazel::CreateApplication();      //extern that will be found in DevGround.cpp
	
int main(int argc, char** argv)
{
	Hazel::Log::Init();
	HZ_CORE_WARN("dick fell off!!!!!!!!!!!!!");
	int a = 5;
	HZ_INFO("Var={0}", a);


	auto app = Hazel::CreateApplication();                 //creates instance of applicaiton
	app->Run();
	delete app;
}

#endif


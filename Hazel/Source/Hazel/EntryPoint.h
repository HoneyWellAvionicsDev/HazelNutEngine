#pragma once

#ifdef HZ_PLATFORM_WINDOWS

extern Hazel::Application* Hazel::CreateApplication();      //extern that will be found in DevGround.cpp
	
int main(int argc, char** argv)
{
	auto app = Hazel::CreateApplication();                 //creates instance of applicaiton
	app->Run();
	delete app;
}

#endif


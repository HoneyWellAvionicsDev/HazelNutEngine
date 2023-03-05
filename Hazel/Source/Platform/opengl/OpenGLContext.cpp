#include <hzpch.h>
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include "OpenGLContext.h"

namespace Jbonk
{
	OpenGLContext::OpenGLContext(GLFWwindow* windowHandle)
		: m_WindowHandle(windowHandle)
	{
		JB_CORE_ASSERT(windowHandle, "Window handle is null!")
	}

	void OpenGLContext::Init()
	{
		PROFILE_FUNCTION();

		glfwMakeContextCurrent(m_WindowHandle);                                                     //sets our render target to be our window
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);                      //gets glfw extensions 
		JB_CORE_ASSERT(status, "Failed to initialize Glad!");

		JB_CORE_INFO("OpenGL Info:");
		JB_CORE_INFO("  Vendor: {0}", glGetString(GL_VENDOR));
		JB_CORE_INFO("  Graphics Card: {0}", glGetString(GL_RENDERER));
		JB_CORE_INFO("  Version: {0}", glGetString(GL_VERSION));


		#ifdef ENABLE_ASSERTS
			int versionMajor;
			int versionMinor;
			glGetIntegerv(GL_MAJOR_VERSION, &versionMajor);
			glGetIntegerv(GL_MINOR_VERSION, &versionMinor);
		
			JB_CORE_ASSERT(versionMajor > 4 || (versionMajor == 4 && versionMinor >= 5), "Jbonk requires at least OpenGL version 4.5!");
		#endif
	}

	void OpenGLContext::SwapBuffers()
	{
		PROFILE_FUNCTION();
		glfwSwapBuffers(m_WindowHandle);
	}

}
#include "hzpch.h"
#include "Hazel/Core/Input.h"
#include "Hazel/Core/Application.h"

#include <GLFW/glfw3.h>

namespace Hazel
{
	bool Input::IsKeyPressed(int keycode)
	{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());            //gets our application window as a void pointer and casts it into a GLFWwindow pointer
		auto state = glfwGetKey(window, keycode);
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}
	bool Input::IsMouseButtonPressed(int button)
	{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		auto state = glfwGetMouseButton(window, button);
		return state == GLFW_PRESS;

	}
	std::pair<float, float> Input::GetMousePosition()
	{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		return std::pair<float, float>(xpos, ypos);
	}
	float Input::GetMouseX()
	{
		auto [x, y] = GetMousePosition();               // [names the values this func returns]
		return x;
	}

	float Input::GetMouseY()
	{
		auto [x, y] = GetMousePosition();
		return y;
	}
}

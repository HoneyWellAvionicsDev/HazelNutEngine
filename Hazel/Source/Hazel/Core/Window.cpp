#include <hzpch.h>
#include "Hazel/Core/Window.h"

#ifdef HZ_PLATFORM_WINDOWS
#include "Platform/Windows/WindowsWindow.h"
#endif

namespace Jbonk
{
	Scope<Window> Window::Create(const WindowProps& props)
	{
#ifdef HZ_PLATFORM_WINDOWS
		return CreateScope<WindowsWindow>(props);
#else
		JB_CORE_ASSERT(false, "Unknown platform!");
		return nullptr;
#endif
	}
}
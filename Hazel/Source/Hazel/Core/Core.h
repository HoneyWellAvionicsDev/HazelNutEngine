#pragma once

#include "Hazel/Core/PlatformDetection.h"

#include <memory>

#ifdef HZ_DEBUG
	#if defined(HZ_PLATFORM_WINDOWS)
		#define DEBUG_BREAK() __debugbreak()
	#elif defined(HZ_PLATFORM_LINUX)
		#include <signal.h>
		#define DEBUG_BREAK() raise(SIGTRAP)
	#else
		#error "Platform doesn't support debugbreak yet"
	#endif
	#define ENABLE_ASSERTS
#else
	#define DEBUG_BREAK()
#endif

#define EXPAND_MACRO(x) x
#define STRINGIFY_MACRO(x) #x

#define BIT(x) (1 << x)

#define BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

#define FORCEINLINE __forceinline

namespace Jbonk
{
	template<typename T>
	using Scope = std::unique_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	using Ref = std::shared_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}
}

#include "Hazel/Core/Assert.h"
#include "Hazel/Core/Log.h"
#include "Hazel/Core/Exception.h"




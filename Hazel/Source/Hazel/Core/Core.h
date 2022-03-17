#pragma once

#include <memory>

#ifdef _WIN32
	#ifdef _WIN64
		#define HZ_PLATFORM_WINDOWS
	#else
		#error "x86 buikds are not supported. Get a modern system."
	#endif
#elif defined(__APPLE__) || defined(__MACH__)
	#include <TargetConditionals.h>
	#if TARGET_IPHONE_SIMULATOR == 1
		#error "IOS simulator is not supported."
	#elif TARGET_OS_PHONE
		#define HZ_PLATFORM_IOS
		#error "IOS is not supported. Stop wasting your money on apple products they're dogshit."
	#elif TARGET_OS_MAC == 1
		#define HZ_PLATFORM_MACOS
		#error "MacOS is not supported"
	#else
		#error "Unknown Apple Platform"
	#endif
#elif defined(__ANDROID__)
	#define HZ_PLATFORN_ANDROID
	#error "Android is not currently supported yet"
#elif defined(__linux__)
	#define HZ_PLATFORM_LINUX
#else
	#define HAZEL_API
#endif

#ifdef HZ_DEBUG
	#if defined(HZ_PLATFORM_WINDOWS)
		#define HZ_DEBUGBREAK() __debugbreak()
	#elif defined(HZ_PLATFORM_LINUX)
		#include <signal.h>
		#define HZ_DEBUGBREAK() raise(SIGTRAP)
	#else
		#error "Platform doesn't support debugbreak yet"
	#endif
	#define HZ_ENABLE_ASSERTS
#else
	#define HZ_DEBUGBREAK()
#endif

#ifdef HZ_PLATFORM_WINDOWS
	#if HZ_DYNAMIC_LINK
		#ifdef HZ_BUILD_DLL
			#define HAZEL_API __declspec(dllexport)
		#else
			#define HAZEL_API __declspec(dllimport)
		#endif
	#else
		#define HAZEL_API
	#endif
#else
	#error Hazel only supports windows even though its a dogshit operationg system
#endif

//todo: assert macros with only condition argument
#ifdef HZ_ENABLE_ASSERTS
	#define HZ_ASSERT(x, ...)	{ if(!(x)) { HZ_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
	#define HZ_CORE_ASSERT(x, ...) { if(!(x)) { HZ_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#else
	#define HZ_ASSERT(x, ...)
	#define HZ_CORE_ASSERT(x, ...)
#endif

#define BIT(x) (1 << x)

#define HZ_BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

namespace Hazel
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




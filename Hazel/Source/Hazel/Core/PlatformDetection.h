#pragma once

#ifdef _WIN32
#ifdef _WIN64
#define HZ_PLATFORM_WINDOWS
#else
#error "x86 builds are not supported. Get a modern system."
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
#endif
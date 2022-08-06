#pragma once

#include "Hazel/Core/PlatformDetection.h"

#ifdef HZ_PLATFORM_WINDOWS
	#ifndef NOMINMAX
		#define NOMINMAX 
	#endif
#endif

#include <iostream>
#include <memory>
#include <utility>
#include <chrono>
#include <thread>
#include <algorithm>
#include <functional>

#include <string>
#include <string_view>
#include <sstream>
#include <fstream>
#include <array>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <atomic>

#include "Hazel/Core/Core.h"
#include "Hazel/Core/Log.h"
#include "Hazel/Core/Timer.h"
#include "Hazel/Core/KeyCodes.h"
#include "Hazel/Debug/Instrumentor.h"

#ifdef HZ_PLATFORM_WINDOWS
	#include <Windows.h>
#endif
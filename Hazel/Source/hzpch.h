#pragma once



#include <iostream>
#include <memory>
#include <utility>
#include <chrono>
#include <thread>
#include <algorithm>
#include <functional>

#include <string>
#include <sstream>
#include <array>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <atomic>


#include "Hazel/Core/Log.h"
#include "Hazel/Core/Timer.h"
#include "Hazel/Debug/Instrumentor.h"

#ifdef HZ_PLATFORM_WINDOWS
	#include <Windows.h>
#endif
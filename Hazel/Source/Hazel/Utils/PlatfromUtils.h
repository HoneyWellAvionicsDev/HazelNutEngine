#pragma once

#include <string>

namespace Hazel
{
	class FileDialogs
	{
	public:
		//returns empty string if cancelled
		static std::string OpenFile(const char* filter);
		static std::string SaveFile(const char* filter);
	};
}

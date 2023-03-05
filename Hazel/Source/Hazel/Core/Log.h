#pragma once

#include "Core.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

//This ignores all warnings raised inside external headers
#pragma warning(push, 0)
#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"
#pragma warning(pop)

namespace Jbonk
{
	class Log
	{
	public:
		static void Init();

		inline static Ref<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static Ref<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
	private:
		static Ref<spdlog::logger> s_CoreLogger;
		static Ref<spdlog::logger> s_ClientLogger;
	};
}

template<typename OStream, glm::length_t L, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, const glm::vec<L, T, Q>& vector)
{
	return os << glm::to_string(vector);
}
template<typename OStream, glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, const glm::mat<C, R, T, Q>& matrix)
{
	return os << glm::to_string(matrix);
}

template<typename OStream, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, glm::qua<T, Q> quaternion)
{
	return os << glm::to_string(quaternion);
}

//Core log macros
#define JB_CORE_TRACE(...)		::Jbonk::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define JB_CORE_INFO(...)		::Jbonk::Log::GetCoreLogger()->info(__VA_ARGS__)
#define JB_CORE_WARN(...)		::Jbonk::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define JB_CORE_ERROR(...)		::Jbonk::Log::GetCoreLogger()->error(__VA_ARGS__)
#define JB_CORE_FATAL(...)		::Jbonk::Log::GetCoreLogger()->critical(__VA_ARGS__)

//Client log macros
#define JB_TRACE(...)			::Jbonk::Log::GetClientLogger()->trace(__VA_ARGS__)
#define JB_INFO(...)			::Jbonk::Log::GetClientLogger()->info(__VA_ARGS__)
#define JB_WARN(...)			::Jbonk::Log::GetClientLogger()->warn(__VA_ARGS__)
#define JB_ERROR(...)			::Jbonk::Log::GetClientLogger()->error(__VA_ARGS__)
#define JB_FATAL(...)			::Jbonk::Log::GetClientLogger()->critical(__VA_ARGS__)

//if distrubution build
//#define HZ_CORE_INFO

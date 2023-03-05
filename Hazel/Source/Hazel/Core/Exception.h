#pragma once

#include <string>
#include <sstream>
#include <exception>

#define EXCEPT_INFO __FUNCTION__,__FILE__,__LINE__

#define EXCEPTION(Base, Current)																     	   \
class Current : public Base																		     		\
{																								     		 \
public:																							     		  \
	Current(const std::string& description, const std::string& function = "?", const std::string& file = "?",  \
			const int line = -1, const std::string& name=#Current)												\
		: Base (description, function, file, line, name) {}													     \
}

#define RAISE(Error, Message) { std::ostringstream oss; oss << Message; throw(Error(oss.str(), EXCEPT_INFO)); }

namespace Jbonk
{
	class Exception : public std::exception
	{
	public:
		Exception(const std::string& description, const std::string& function, const std::string& file, const int line, const std::string& name = "Exception")
			: _Description(description), _Function(function), _File(file), _Line(line), _Name(name)
		{
			std::ostringstream message;
			message << '<' << _Name << ">" << _Description << "\t\t" << _Function << " @ " << _File << ":" << _Line << "";
			_Message = message.str();
		}

		virtual ~Exception() noexcept {}

		const char* what() const noexcept { return _Message.c_str(); }
	private:
		std::string _Description;
		std::string _Function;
		std::string _File;
		std::string _Name;
		std::string _Message;
		int _Line;
	};
}

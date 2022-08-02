#pragma once

#include <string>
#include <filesystem>
#include "Hazel/Core/Core.h"

namespace Hazel
{
	class Texture
	{
	public:
		virtual ~Texture() = default;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual uint32_t GetRendererID() const = 0;
		virtual std::string GetPath() const = 0;

		virtual void SetData(void* data, uint32_t size) = 0;
		virtual void SetPath(const std::string& path) = 0;
		virtual void Bind(uint32_t slot = 0) const = 0;
		virtual bool operator==(const Texture& other) const = 0;
	};

	class Texture2D : public Texture
	{
	public:
		static Ref<Texture2D> Upload(const std::string& path);		  //change name to Create to match our API
		static Ref<Texture2D> Upload(uint32_t width, uint32_t height);//
	};
}



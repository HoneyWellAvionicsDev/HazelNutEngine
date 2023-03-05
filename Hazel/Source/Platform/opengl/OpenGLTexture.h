#pragma once

#include "Hazel/Renderer/Texture.h"
#include <glad/glad.h>

namespace Jbonk
{
	class OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D(const std::string& path);
		OpenGLTexture2D(uint32_t width, uint32_t height);
		virtual ~OpenGLTexture2D();

		virtual uint32_t GetWidth() const override { return m_Width; }
		virtual uint32_t GetHeight() const override { return m_Height; }
		virtual uint32_t GetRendererID() const override { return m_RendererID; }
		virtual std::string GetPath() const override { return m_Path; }

		virtual void SetPath(const std::string& path) override { m_Path = path; }
		virtual void SetData(void* data, uint32_t size) override;
		virtual void Bind(uint32_t slot = 0) const override;
		virtual bool operator==(const Texture& other) const override { return m_RendererID == other.GetRendererID(); }

	private:
		std::string m_Path;
		uint32_t m_Width;
		uint32_t m_Height;
		uint32_t m_RendererID;
		GLenum m_InteralFormat;
		GLenum m_Format;
	};
}


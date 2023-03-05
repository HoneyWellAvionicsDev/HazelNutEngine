#include "hzpch.h"
#include "OpenGLTexture.h"

#include <stb_image.h>


namespace Jbonk
{
	OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height)
		: m_Width(width), m_Height(height)
	{
		PROFILE_FUNCTION();

		m_InteralFormat = GL_RGBA8;
		m_Format = GL_RGBA;
		
		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);                                         
		glTextureStorage2D(m_RendererID, 1, m_InteralFormat, m_Width, m_Height);

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);                          
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	OpenGLTexture2D::OpenGLTexture2D(const std::string& path)
		: m_Path(path)
	{
		PROFILE_FUNCTION();
		//TODO: add a check to see if file path exsists
		int width, height, channels;
		stbi_set_flip_vertically_on_load(1);
		stbi_uc* data = nullptr;
		{
			PROFILE_SCOPE("OpenGLTexture2D::OpenGLTexture2D(const std::string&) - stbi_load");
			Timer timer;
			data = stbi_load(path.c_str(), &width, &height, &channels, 0);                        //stores texture on RAM
			JB_CORE_TRACE("Texture from path {0} took {1}ms", path, timer.ElapsedMilliseconds());
		}
		JB_CORE_ASSERT(data, "Failed to load image!"); //maybe we could throw after asserting
		m_Width = width;
		m_Height = height;

		GLenum internalFormat = 0, dataFormat = 0;
		if (channels == 4)
		{
			internalFormat = GL_RGBA8;
			dataFormat = GL_RGBA;
		}
		else if (channels == 3)
		{
			internalFormat = GL_RGB8;
			dataFormat = GL_RGB;
		}

		m_InteralFormat = internalFormat;
		m_Format = dataFormat;

		JB_CORE_ASSERT(internalFormat, "Format not supported yet!");

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);                                             //create texture on GPU
		glTextureStorage2D(m_RendererID, 1, internalFormat, m_Width, m_Height);                               //create space for texture on GPU

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);                           //texture params such as Min/Magnification and mipmapping
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, dataFormat, GL_UNSIGNED_BYTE, data); //upload texture to GPU

		stbi_image_free(data);
	}


	OpenGLTexture2D::~OpenGLTexture2D()
	{
		PROFILE_FUNCTION();

		glDeleteTextures(1, &m_RendererID);
	}

	void OpenGLTexture2D::SetData(void* data, uint32_t size)
	{
		PROFILE_FUNCTION();

		uint32_t bpp = m_Format == GL_RGBA ? 4 : 3;
		JB_CORE_ASSERT(size == m_Width * m_Height * bpp, "Data must be entire texture!");
		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_Format, GL_UNSIGNED_BYTE, data);
	}

	void OpenGLTexture2D::Bind(uint32_t slot) const
	{
		glBindTextureUnit(slot, m_RendererID);                                                            //binds texture to slot (unit)
	}
}

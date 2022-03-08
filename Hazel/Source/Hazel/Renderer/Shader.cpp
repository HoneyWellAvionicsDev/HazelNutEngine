#include "hzpch.h"
#include "Shader.h"
#include "Renderer.h"
#include "Platform/opengl/OpenGLShader.h"

#include <glad/glad.h>

namespace Hazel
{
	Ref<Shader> Shader::Upload(const std::string& filepath)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:			HZ_CORE_ASSERT(false, "You need a renderer API dumbass"); return nullptr;
		case RendererAPI::API::OpenGL:			return make_shared<OpenGLShader>(filepath);
		}

		HZ_CORE_ASSERT(false, "Hazel failed to detect the renderer API for unknown reasons");
		return nullptr;
	}

	Ref<Shader> Shader::Upload(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:			HZ_CORE_ASSERT(false, "You need a renderer API dumbass"); return nullptr;
		case RendererAPI::API::OpenGL:			return make_shared<OpenGLShader>(name, vertexSrc, fragmentSrc);
		}

		HZ_CORE_ASSERT(false, "Hazel failed to detect the renderer API for unknown reasons");
		return nullptr;
	}

	void ShaderLibrary::Add(const std::string& name, const Ref<Shader>& shader)
	{
		HZ_CORE_ASSERT(!Exsits(name), "Shader already exists!");
		m_Shaders[name] = shader;
	}

	void ShaderLibrary::Add(const Ref<Shader>& shader)
	{
		auto& name = shader->GetName();
		Add(name, shader);
	}

	Ref<Shader> ShaderLibrary::Load(const std::string& filepath)
	{
		auto shader = Shader::Upload(filepath);
		Add(shader);
		return shader;
	}

	Ref<Shader> ShaderLibrary::Load(const std::string& name, const std::string& filepath)
	{
		auto shader = Shader::Upload(filepath);
		Add(name, shader);
		return shader;
	}

	Ref<Shader> ShaderLibrary::Get(const std::string& name)
	{
		HZ_CORE_ASSERT(Exsits(name), "Shader not found!");
		return m_Shaders[name];
	}
	bool ShaderLibrary::Exsits(const std::string& name) const
	{
		return m_Shaders.find(name) != m_Shaders.end();
	}
}

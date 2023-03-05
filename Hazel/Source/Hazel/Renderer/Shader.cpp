#include "hzpch.h"
#include "Shader.h"
#include "Renderer.h"
#include "Platform/opengl/OpenGLShader.h"

#include <glad/glad.h>

namespace Jbonk
{
	Ref<Shader> Shader::Upload(const std::string& filepath)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:			JB_CORE_ASSERT(false, "You need a renderer API dumbass"); return nullptr;
		case RendererAPI::API::OpenGL:			return CreateRef<OpenGLShader>(filepath);
		}

		JB_CORE_ASSERT(false, "Jbonk failed to detect the renderer API for unknown reasons");
		return nullptr;
	}

	Ref<Shader> Shader::Upload(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:			JB_CORE_ASSERT(false, "You need a renderer API dumbass"); return nullptr;
		case RendererAPI::API::OpenGL:			return CreateRef<OpenGLShader>(name, vertexSrc, fragmentSrc);
		}

		JB_CORE_ASSERT(false, "Jbonk failed to detect the renderer API for unknown reasons");
		return nullptr;
	}
 
	void ShaderLibrary::Add(const std::string& name, const Ref<Shader>& shader)
	{
		JB_CORE_ASSERT(!Exsits(name), "Shader already exists!");
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
		JB_CORE_ASSERT(Exsits(name), "Shader not found!");
		return m_Shaders[name];
	}
	bool ShaderLibrary::Exsits(const std::string& name) const
	{
		return m_Shaders.find(name) != m_Shaders.end();
	}
}

#pragma once

#include "Hazel.h"
#include "Hazel/Core/Layer.h"


class Sandbox2D : public Hazel::Layer
{
public:
	Sandbox2D();
	virtual ~Sandbox2D() = default;

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(Hazel::Timestep ts) override;
	void OnImGuiRender() override;
	void OnEvent(Hazel::Event& event) override;

private:
	Hazel::OrthographicCameraController m_CameraController;

	//temp
	Hazel::Ref<Hazel::VertexArray> m_SquareVA;
	Hazel::Ref<Hazel::Shader> m_FlatColorShader;
	Hazel::Ref<Hazel::Texture2D> m_Texture;
	Hazel::Ref<Hazel::Texture2D> m_Texture2;
	Hazel::Ref<Hazel::Texture2D> m_SpriteSheet;
	Hazel::Ref<Hazel::SubTexture2D> m_FullHeart;

	glm::vec4 m_SquareColor = { 0.2f, 0.3f, 0.8f, 1.f };
	float m_RotationalSpeed = 60.f;

	std::unordered_map<char, Hazel::Ref<Hazel::SubTexture2D>> s_TextureMap;
};


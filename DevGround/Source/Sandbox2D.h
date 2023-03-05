#pragma once

#include "Hazel.h"
#include "Hazel/Core/Layer.h"


class Sandbox2D : public Jbonk::Layer
{
public:
	Sandbox2D();
	virtual ~Sandbox2D() = default;

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(Jbonk::Timestep ts) override;
	void OnImGuiRender() override;
	void OnEvent(Jbonk::Event& event) override;

private:
	Jbonk::OrthographicCameraController m_CameraController;

	//temp
	Jbonk::Ref<Jbonk::VertexArray> m_SquareVA;
	Jbonk::Ref<Jbonk::Shader> m_FlatColorShader;
	Jbonk::Ref<Jbonk::Texture2D> m_Texture;
	Jbonk::Ref<Jbonk::Texture2D> m_Texture2;
	Jbonk::Ref<Jbonk::Texture2D> m_SpriteSheet;
	Jbonk::Ref<Jbonk::SubTexture2D> m_FullHeart;

	glm::vec4 m_SquareColor = { 0.2f, 0.3f, 0.8f, 1.f };
	float m_RotationalSpeed = 60.f;

	std::unordered_map<char, Jbonk::Ref<Jbonk::SubTexture2D>> s_TextureMap;
};


#include "Sandbox2D.h"
#include <imgui.h>
#include "glm/gtc/type_ptr.hpp"


Sandbox2D::Sandbox2D()
	: Layer("Sandbox2D"), m_CameraController(1280.f / 720.f, true)
{
}

void Sandbox2D::OnAttach()
{
	HZ_PROFILE_FUNCTION();
	m_Texture = Hazel::Texture2D::Upload("assets/textures/Space.png");
	m_SpriteSheet = Hazel::Texture2D::Upload("assets/game/tiles_packed.png");
	m_FullHeart = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, {17, 2}, { 18, 18 }, {1, 3});
}

void Sandbox2D::OnDetach()
{
	HZ_PROFILE_FUNCTION();
}

void Sandbox2D::OnUpdate(Hazel::Timestep ts)
{
	HZ_PROFILE_FUNCTION();

	//------------------Update----------------------------------
	m_CameraController.OnUpdate(ts);

	//------------------Render----------------------------------
	Hazel::Renderer2D::ResetStats();
	Hazel::RenderCommand::SetClearColor({ 0.04f, 0.04f, 0.04f, 1 });
	Hazel::RenderCommand::Clear();

	Hazel::Renderer2D::BeginScene(m_CameraController.GetCamera());

	Hazel::Renderer2D::DrawQuad({ -1.f, 0.f }, { 0.8f, 0.8f }, { 0.8f, 1.f, 0.9f, 0.8f });
	Hazel::Renderer2D::DrawQuad({ 0.5f, -0.5f }, { 0.5f, 0.75f }, { 0.8f, 0.67f, 0.12f, 0.8f });

	static float rotation = 0.f;
	rotation += glm::radians(ts * m_RotationalSpeed);

	Hazel::Renderer2D::DrawRotatedQuad({ 0.f, 0.f, -0.8f}, { 15.0f, 45.0f }, rotation + 30.f, m_FullHeart, m_SquareColor);
	Hazel::Renderer2D::DrawRotatedQuad({ 9.5f, -19.5f, -0.9 }, { 5.5f, 9.5f }, rotation, m_SquareColor);
	Hazel::Renderer2D::DrawRotatedQuad({ 5.f, 7.f, -0.99f }, { 175.5f, 175.5f }, 45.f, m_Texture, { 0.3f, 0.97f, 0.62f, 0.95f });

	for (float y = -5.f; y < 5.f; y += 0.4f)
	{
		for (float x = -5.f; x < 5.f; x += 0.4f)
		{
			glm::vec4 color = { (x + 5.f) / 10.f, 0.4f, (y + 5.f) / 10.f, 0.4f };
			Hazel::Renderer2D::DrawQuad({ x,y }, { 0.45f, 0.45f }, color);
		}
	}

	Hazel::Renderer2D::EndScene();
}

void Sandbox2D::OnImGuiRender()
{
	HZ_PROFILE_FUNCTION();

	ImGui::Begin("Settings");
	auto stats = Hazel::Renderer2D::GetStats();
	ImGui::Text("Renderer2D Stats: ");
	ImGui::Text("Draw Calls: %d", stats.DrawCalls);
	ImGui::Text("Quads: %d", stats.QuadCount);
	ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
	ImGui::Text("Indices: %d", stats.GetTotalIndexCount());

	ImGui::ColorEdit4("Square Color", glm::value_ptr(m_SquareColor));
	ImGui::End();
}

void Sandbox2D::OnEvent(Hazel::Event& event)
{
	m_CameraController.OnEvent(event);
}

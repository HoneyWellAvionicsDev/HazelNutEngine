#include "Sandbox2D.h"
#include <imgui.h>
#include "glm/gtc/type_ptr.hpp"


static const char* s_MapTiles =
"GGGGWWWWWWWWWWGGGGGGGGGGWWWGGGGGGGGGGGWWGGGGGGGG"
"DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD"
"DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD"
"DDDDDDDDDDDDDDDDDDDDDDEDDDDDDDDDDDDDDDDDDDDDDDDD"
"DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD"
"DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD"
"DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD"
"DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD";

Sandbox2D::Sandbox2D()
	: Layer("Sandbox2D"), m_CameraController(1280.f / 720.f)
{
}

void Sandbox2D::OnAttach()
{
	HZ_PROFILE_FUNCTION();
	m_Texture = Hazel::Texture2D::Upload("assets/textures/Space.png");
	m_Texture2 = Hazel::Texture2D::Upload("assets/textures/purple-square-9.png");
	m_SpriteSheet = Hazel::Texture2D::Upload("assets/game/tiles_packed.png");
	s_TextureMap['G'] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, {18, 8}, {18, 18}, {1, 1});
	s_TextureMap['D'] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, {2, 2}, {18, 18}, {1, 1});
	s_TextureMap['W'] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 13, 6 }, { 18, 18 }, { 1, 1 });
	m_FullHeart = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, {2, 1}, { 18, 18 }, {1, 1});

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
#if 1
	Hazel::Renderer2D::DrawQuad({ -1.f, 0.f }, { 0.8f, 0.8f }, { 0.8f, 1.f, 0.9f, 0.8f });
	Hazel::Renderer2D::DrawQuad({ 0.5f, -0.5f }, { 0.5f, 0.75f }, { 0.8f, 0.67f, 0.12f, 0.8f });
	Hazel::Renderer2D::DrawQuad({ 9.5f, -4.5f }, { 5.5f, 5.5f }, m_FullHeart);

	static float rotation = 0.f;
	rotation += glm::radians(ts * m_RotationalSpeed);

	//Hazel::Renderer2D::DrawRotatedQuad({ 5.f, 7.f, -0.99f }, { 175.5f, 175.5f }, 45.f, m_Texture, { 0.3f, 0.97f, 0.62f, 0.95f });
	Hazel::Renderer2D::DrawRotatedQuad({ 9.5f, -19.5f, -0.9 }, { 5.5f, 5.5f }, rotation, m_SquareColor);
	Hazel::Renderer2D::DrawRotatedQuad({ 0.f, 0.f, -0.8f}, { 15.0f, 15.0f }, rotation + 30.f, m_Texture2, m_SquareColor);

	for (float y = -5.f; y < 5.f; y += 0.4f)
	{
		for (float x = -5.f; x < 5.f; x += 0.4f)
		{
			glm::vec4 color = { (x + 5.f) / 10.f, 0.4f, (y + 5.f) / 10.f, 0.4f };
			Hazel::Renderer2D::DrawQuad({ x,y }, { 0.45f, 0.45f }, color);
		}
	}
#endif
#if 1
	for (uint32_t y = 0; y < 8; y++)
	{
		for (uint32_t x = 0; x < 48; x++)
		{
			char tileType = s_MapTiles[x + y * 48];
			Hazel::Ref<Hazel::SubTexture2D> texture;
			if (s_TextureMap.find(tileType) != s_TextureMap.end())
				texture = s_TextureMap[tileType];
			else
				texture = m_FullHeart;
			Hazel::Renderer2D::DrawQuad({ x , 8 - y, 1}, { 1.f, 1.f }, texture);
		}
	}
#endif
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

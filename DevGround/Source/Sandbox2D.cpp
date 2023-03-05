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
	PROFILE_FUNCTION();
	m_Texture = Jbonk::Texture2D::Upload("assets/textures/Space.png");
	m_Texture2 = Jbonk::Texture2D::Upload("assets/textures/purple-square-9.png");
	m_SpriteSheet = Jbonk::Texture2D::Upload("assets/game/tiles_packed.png");
	s_TextureMap['G'] = Jbonk::SubTexture2D::CreateFromCoords(m_SpriteSheet, {18, 8}, {18, 18}, {1, 1});
	s_TextureMap['D'] = Jbonk::SubTexture2D::CreateFromCoords(m_SpriteSheet, {2, 2}, {18, 18}, {1, 1});
	s_TextureMap['W'] = Jbonk::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 13, 6 }, { 18, 18 }, { 1, 1 });
	m_FullHeart = Jbonk::SubTexture2D::CreateFromCoords(m_SpriteSheet, {2, 1}, { 18, 18 }, {1, 1});

}

void Sandbox2D::OnDetach()
{
	PROFILE_FUNCTION();
}

void Sandbox2D::OnUpdate(Jbonk::Timestep ts)
{
	PROFILE_FUNCTION();

	//------------------Update----------------------------------
	m_CameraController.OnUpdate(ts);

	//------------------Render----------------------------------
	Jbonk::Renderer2D::ResetStats();
	Jbonk::RenderCommand::SetClearColor({ 0.04f, 0.04f, 0.04f, 1 });
	Jbonk::RenderCommand::Clear();

	Jbonk::Renderer2D::BeginScene(m_CameraController.GetCamera());
#if 1
	Jbonk::Renderer2D::DrawQuad({ -1.f, 0.f }, { 0.8f, 0.8f }, { 0.8f, 1.f, 0.9f, 0.8f });
	Jbonk::Renderer2D::DrawQuad({ 0.5f, -0.5f }, { 0.5f, 0.75f }, { 0.8f, 0.67f, 0.12f, 0.8f });
	Jbonk::Renderer2D::DrawQuad({ 9.5f, -4.5f }, { 5.5f, 5.5f }, m_FullHeart);

	static float rotation = 0.f;
	rotation += glm::radians(ts * m_RotationalSpeed);

	//Jbonk::Renderer2D::DrawRotatedQuad({ 5.f, 7.f, -0.99f }, { 175.5f, 175.5f }, 45.f, m_Texture, { 0.3f, 0.97f, 0.62f, 0.95f });
	Jbonk::Renderer2D::DrawRotatedQuad({ 9.5f, -19.5f, -0.9 }, { 5.5f, 5.5f }, rotation, m_SquareColor);
	Jbonk::Renderer2D::DrawRotatedQuad({ 0.f, 0.f, -0.8f}, { 15.0f, 15.0f }, rotation + 30.f, m_Texture2, m_SquareColor);

	for (float y = -5.f; y < 5.f; y += 0.4f)
	{
		for (float x = -5.f; x < 5.f; x += 0.4f)
		{
			glm::vec4 color = { (x + 5.f) / 10.f, 0.4f, (y + 5.f) / 10.f, 0.4f };
			Jbonk::Renderer2D::DrawQuad({ x,y }, { 0.45f, 0.45f }, color);
		}
	}
#endif
#if 1
	for (uint32_t y = 0; y < 8; y++)
	{
		for (uint32_t x = 0; x < 48; x++)
		{
			char tileType = s_MapTiles[x + y * 48];
			Jbonk::Ref<Jbonk::SubTexture2D> texture;
			if (s_TextureMap.find(tileType) != s_TextureMap.end())
				texture = s_TextureMap[tileType];
			else
				texture = m_FullHeart;
			Jbonk::Renderer2D::DrawQuad({ x , 8 - y, 1}, { 1.f, 1.f }, texture);
		}
	}
#endif
	Jbonk::Renderer2D::EndScene();

}

void Sandbox2D::OnImGuiRender()
{
	PROFILE_FUNCTION();
    
    ImGui::Begin("Settings");
    auto stats = Jbonk::Renderer2D::GetStats();
    ImGui::Text("Renderer2D Stats: ");
    ImGui::Text("Draw Calls: %d", stats.DrawCalls);
    ImGui::Text("Quads: %d", stats.QuadCount);
    ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
    ImGui::Text("Indices: %d", stats.GetTotalIndexCount());

    ImGui::ColorEdit4("Square Color", glm::value_ptr(m_SquareColor));

    ImGui::End();
}

void Sandbox2D::OnEvent(Jbonk::Event& event)
{
	m_CameraController.OnEvent(event);
}

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
	: Layer("Sandbox2D"), m_CameraController(1280.f / 720.f, true)
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
#if 1
	Hazel::Renderer2D::DrawQuad({ -1.f, 0.f }, { 0.8f, 0.8f }, { 0.8f, 1.f, 0.9f, 0.8f });
	Hazel::Renderer2D::DrawQuad({ 0.5f, -0.5f }, { 0.5f, 0.75f }, { 0.8f, 0.67f, 0.12f, 0.8f });
	Hazel::Renderer2D::DrawQuad({ 9.5f, -4.5f }, { 5.5f, 5.5f }, m_FullHeart);

	static float rotation = 0.f;
	rotation += glm::radians(ts * m_RotationalSpeed);

	Hazel::Renderer2D::DrawRotatedQuad({ 0.f, 0.f, -0.8f}, { 15.0f, 15.0f }, rotation + 30.f, m_Texture2, m_SquareColor);
	Hazel::Renderer2D::DrawRotatedQuad({ 9.5f, -19.5f, -0.9 }, { 5.5f, 5.5f }, rotation, m_SquareColor);
	Hazel::Renderer2D::DrawRotatedQuad({ 5.f, 7.f, -0.99f }, { 175.5f, 175.5f }, 45.f, m_Texture, { 0.3f, 0.97f, 0.62f, 0.95f });

	for (float y = -5.f; y < 5.f; y += 0.4f)
	{
		for (float x = -5.f; x < 5.f; x += 0.4f)
		{
			glm::vec4 color = { (x + 5.f) / 10.f, 0.4f, (y + 5.f) / 10.f, 0.4f };
			Hazel::Renderer2D::DrawQuad({ x,y }, { 0.45f, 0.45f }, color);
		}
	}
#endif
	Hazel::Renderer2D::EndScene();
}

void Sandbox2D::OnImGuiRender()
{
	HZ_PROFILE_FUNCTION();

	static bool dockspaceOpen = true;
    static bool opt_fullscreen = true;
    static bool opt_padding = false;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
    // because it would be confusing to have two docking targets within each others.
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    if (opt_fullscreen)
    {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    }
    else
    {
        dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
    }

    // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
    // and handle the pass-thru hole, so we ask Begin() to not render a background.
    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;

    // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
    // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
    // all active windows docked into it will lose their parent and become undocked.
    // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
    // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
    if (!opt_padding)
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
    if (!opt_padding)
        ImGui::PopStyleVar();

    if (opt_fullscreen)
        ImGui::PopStyleVar(2);

    // Submit the DockSpace
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    }

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            // Disabling fullscreen would allow the window to be moved to the front of other windows,
            // which we can't undo at the moment without finer window depth/z control.
            ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen);
            ImGui::MenuItem("Padding", NULL, &opt_padding);
            ImGui::Separator();

            if (ImGui::MenuItem("Quit")) Hazel::Application::Get().CloseWindow();
            //if (ImGui::MenuItem("Flag: NoResize", "", (dockspace_flags & ImGuiDockNodeFlags_NoResize) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoResize; }
            //if (ImGui::MenuItem("Flag: NoDockingInCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_NoDockingInCentralNode) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoDockingInCentralNode; }
            //if (ImGui::MenuItem("Flag: AutoHideTabBar", "", (dockspace_flags & ImGuiDockNodeFlags_AutoHideTabBar) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_AutoHideTabBar; }
            //if (ImGui::MenuItem("Flag: PassthruCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) != 0, opt_fullscreen)) { dockspace_flags ^= ImGuiDockNodeFlags_PassthruCentralNode; }
           
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    ImGui::Begin("Settings");
    auto stats = Hazel::Renderer2D::GetStats();
    ImGui::Text("Renderer2D Stats: ");
    ImGui::Text("Draw Calls: %d", stats.DrawCalls);
    ImGui::Text("Quads: %d", stats.QuadCount);
    ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
    ImGui::Text("Indices: %d", stats.GetTotalIndexCount());

    ImGui::ColorEdit4("Square Color", glm::value_ptr(m_SquareColor));

    uint32_t textureID = m_SpriteSheet->GetRendererID();
    ImGui::Image((void*)textureID, ImVec2{512.f, 512.f});
    ImGui::End();

    ImGui::ShowDemoWindow(&dockspaceOpen);
    
    ImGui::End();
}

void Sandbox2D::OnEvent(Hazel::Event& event)
{
	m_CameraController.OnEvent(event);
}

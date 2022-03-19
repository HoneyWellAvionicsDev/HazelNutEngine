#include "HazelEditorLayer.h"
#include <imgui.h>
#include "glm/gtc/type_ptr.hpp"

namespace Hazel
{
    static const char* s_MapTiles =
    "GGGGWWWWWWWWWWGGGGGGGGGGWWWGGGGGGGGGGGWWGGGGGGGG"
    "DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD"
    "DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD"
    "DDDDDDDDDDDDDDDDDDDDDDEDDDDDDDDDDDDDDDDDDDDDDDDD"
    "DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD"
    "DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD"
    "DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD"
    "DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD";
    
    EditorLayer::EditorLayer()
    	: Layer("EditorLayer"), m_CameraController(1280.f / 720.f, true)
    {
    }
    
    void EditorLayer::OnAttach()
    {
    	HZ_PROFILE_FUNCTION();
    	m_Texture = Texture2D::Upload("assets/textures/Space.png");
    	m_Texture2 = Texture2D::Upload("assets/textures/purple-square-9.png");
    	m_SpriteSheet = Texture2D::Upload("assets/game/tiles_packed.png");
    	s_TextureMap['G'] = SubTexture2D::CreateFromCoords(m_SpriteSheet, {18, 8}, {18, 18}, {1, 1});
    	s_TextureMap['D'] = SubTexture2D::CreateFromCoords(m_SpriteSheet, {2, 2}, {18, 18}, {1, 1});
    	s_TextureMap['W'] = SubTexture2D::CreateFromCoords(m_SpriteSheet, { 13, 6 }, { 18, 18 }, { 1, 1 });
    	m_FullHeart = SubTexture2D::CreateFromCoords(m_SpriteSheet, {2, 1}, { 18, 18 }, {1, 1});
    
        FrameBufferSpecification FrameBufferSpec;
        FrameBufferSpec.Width = 2560;
        FrameBufferSpec.Height = 1440;
        m_FrameBuffer = FrameBuffer::Create(FrameBufferSpec);

        m_Scene = CreateRef<Scene>();
        auto square = m_Scene->CreateEntity("TestSquare");
        square.AddComponent<SpriteRendererComponent>(glm::vec4{0.f, 1.f, 0.f, 1.f});
        m_SquareEntity = square;

        m_CameraEntity = m_Scene->CreateEntity("Camera Test");
        m_CameraEntity.AddComponent<CameraComponent>();

        m_CameraEntity2 = m_Scene->CreateEntity("Camera Entity");
        auto & cc = m_CameraEntity2.AddComponent<CameraComponent>();
        cc.Primary = true;

        class CameraController : public ScriptableEntity
        {
        public:
            void OnCreate() override
            {
                auto& translation = GetComponent<TransformComponent>().Translation;
                translation.x = rand() % 10 - 5.f;

            }

            void OnDestroy() override
            {

            }

            void OnUpdate(Timestep ts) override
            {
                auto& translation = GetComponent<TransformComponent>().Translation;
                float speed = 5.f;

                if (Input::IsKeyPressed(HZ_KEY_A))
                    translation.x -= speed * ts;
                if (Input::IsKeyPressed(HZ_KEY_D))
                    translation.x += speed * ts;
                if (Input::IsKeyPressed(HZ_KEY_W))
                    translation.y += speed * ts;
                if (Input::IsKeyPressed(HZ_KEY_S))
                    translation.y -= speed * ts;
            }

        };

        m_CameraEntity.AddComponent<NativeScriptComponent>().Bind<CameraController>();
        m_CameraEntity2.AddComponent<NativeScriptComponent>().Bind<CameraController>();

        m_SceneHierarchyPanel.SetContext(m_Scene);

    }
    
    void EditorLayer::OnDetach()
    {
    	HZ_PROFILE_FUNCTION();
    }
    
    void EditorLayer::OnUpdate(Timestep ts)
    {
    	HZ_PROFILE_FUNCTION();

        if (FrameBufferSpecification spec = m_FrameBuffer->GetSpecification();
            m_ViewportSize.x > 0.f && m_ViewportSize.y > 0.f &&
            (spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y))
        {
            //m_FrameBuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
            m_CameraController.OnResize(m_ViewportSize.x, m_ViewportSize.y);
            m_Scene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
        }

    	//------------------Update----------------------------------
        if(m_ViewportFocused)
    	    m_CameraController.OnUpdate(ts);
    
    	//------------------Render----------------------------------
    	Renderer2D::ResetStats();
        m_FrameBuffer->Bind();
    	RenderCommand::SetClearColor({ 0.04f, 0.04f, 0.04f, 1 });
    	RenderCommand::Clear();

        //update scene
        m_Scene->OnUpdate(ts);
    #if 0
    	Renderer2D::BeginScene(m_CameraController.GetCamera());
    	Renderer2D::DrawQuad({ -1.f, 0.f }, { 0.8f, 0.8f }, { 0.8f, 1.f, 0.9f, 0.8f });
    	Renderer2D::DrawQuad({ 0.5f, -0.5f }, { 0.5f, 0.75f }, { 0.8f, 0.67f, 0.12f, 0.8f });
    	Renderer2D::DrawQuad({ 9.5f, -4.5f }, { 5.5f, 5.5f }, m_FullHeart);
    
    	static float rotation = 0.f;
    	rotation += glm::radians(ts * m_RotationalSpeed);
    
    	Renderer2D::DrawRotatedQuad({ 9.5f, -19.5f, -0.9 }, { 5.5f, 5.5f }, rotation, m_SquareColor);
    	Renderer2D::DrawRotatedQuad({ 0.f, 0.f, -0.8f}, { 15.0f, 15.0f }, rotation + 30.f, m_Texture2, m_SquareColor);
    
    	for (float y = -5.f; y < 5.f; y += 0.4f)
    	{
    		for (float x = -5.f; x < 5.f; x += 0.4f)
    		{
    			glm::vec4 color = { (x + 5.f) / 10.f, 0.4f, (y + 5.f) / 10.f, 0.4f };
    			Renderer2D::DrawQuad({ x,y }, { 0.45f, 0.45f }, color);
    		}
    	}
    #endif
    #if 0
    	for (uint32_t y = 0; y < 8; y++)
    	{
    		for (uint32_t x = 0; x < 48; x++)
    		{
    			char tileType = s_MapTiles[x + y * 48];
    			Ref<SubTexture2D> texture;
    			if (s_TextureMap.find(tileType) != s_TextureMap.end())
    				texture = s_TextureMap[tileType];
    			else
    				texture = m_FullHeart;
    			Renderer2D::DrawQuad({ x , 8 - y, 1}, { 1.f, 1.f }, texture);
    		}
    	}
    #endif
    	//Renderer2D::EndScene();
        m_FrameBuffer->Unbind();
    }
    
    void EditorLayer::OnImGuiRender()
    {
    	HZ_PROFILE_FUNCTION();
    
    	static bool dockspaceOpen = true;
        static bool opt_fullscreen = true;
        static bool opt_padding = false;
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
      
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
    
        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
            window_flags |= ImGuiWindowFlags_NoBackground;
    
        
        if (!opt_padding)
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
        if (!opt_padding)
            ImGui::PopStyleVar();
    
        if (opt_fullscreen)
            ImGui::PopStyleVar(2);
    
        ImGuiIO& io = ImGui::GetIO();
        ImGuiStyle& style = ImGui::GetStyle();
        float minWinSizeX = style.WindowMinSize.x;
        style.WindowMinSize.x = 370.f;
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        }

        style.WindowMinSize.x = minWinSizeX;
    
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen);
                ImGui::MenuItem("Padding", NULL, &opt_padding);
                ImGui::Separator();
    
                if (ImGui::MenuItem("Quit")) Application::Get().CloseWindow();
                
                ImGui::EndMenu();
            }
    
            ImGui::EndMenuBar();
        }

        m_SceneHierarchyPanel.OnImGuiRender();

        ImGui::Begin("Statistics");
        auto stats = Renderer2D::GetStats();
        ImGui::Text("Renderer2D Stats: ");
        ImGui::Text("Draw Calls: %d", stats.DrawCalls);
        ImGui::Text("Quads: %d", stats.QuadCount);
        ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
        ImGui::Text("Indices: %d", stats.GetTotalIndexCount());
      

        ImGui::ShowDemoWindow(&dockspaceOpen);
        ImGui::ColorEdit4("Square Color", glm::value_ptr(m_SquareColor));
        ImGui::End();
    
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0,0 });
        ImGui::Begin("Viewport");

        m_ViewportFocused = ImGui::IsWindowFocused();
        m_ViewportHovered = ImGui::IsWindowHovered();
        Application::Get().GetImGuiLayer()->BlockEvents(!m_ViewportHovered);

        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
        if (m_ViewportSize != *((glm::vec2*)&viewportPanelSize) && viewportPanelSize.x > 0 && viewportPanelSize.y > 0)
        {
            //m_FrameBuffer->Resize((uint32_t)viewportPanelSize.x, (uint32_t)viewportPanelSize.y); 
            m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };
            m_CameraController.OnResize(viewportPanelSize.x, viewportPanelSize.y);
        }
        uint32_t textureID = m_FrameBuffer->GetColorAttachmentRendererID();
        ImGui::Image((void*)textureID, ImVec2{m_ViewportSize.x, m_ViewportSize.y}, ImVec2{0,1}, ImVec2{1,0});
        ImGui::End();
        ImGui::PopStyleVar();

        ImGui::End();
    }
    
    void EditorLayer::OnEvent(Event& event)
    {
    	m_CameraController.OnEvent(event);
    }
}
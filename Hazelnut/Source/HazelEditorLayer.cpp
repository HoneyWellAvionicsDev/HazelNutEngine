#include "HazelEditorLayer.h"

#include "Hazel/Scene/SceneSerializer.h"
#include "Hazel/Utils/PlatfromUtils.h"
#include "Hazel/Math/Math.h"
#include "Hazel/Core/Timer.h"

#include "glm/gtc/type_ptr.hpp"
#include <imgui.h>
#include "ImGuizmo.h"

namespace Hazel
{
	extern const std::filesystem::path g_AssetPath;


	EditorLayer::EditorLayer()
		: Layer("EditorLayer")
	{
	}
	
	void EditorLayer::OnAttach()
	{
		HZ_PROFILE_FUNCTION();

		FrameBufferSpecification FrameBufferSpec;
		FrameBufferSpec.Attachments = { FrameBufferTextureFormat::RGBA8, FrameBufferTextureFormat::RED_INTEGER, FrameBufferTextureFormat::Depth };
		FrameBufferSpec.Width = 1280;
		FrameBufferSpec.Height = 720;
		m_FrameBuffer = FrameBuffer::Create(FrameBufferSpec);

		m_IconPlay = Texture2D::Upload("Resources/Icons/PlayButton.png");
		m_IconSimulate = Texture2D::Upload("Resources/Icons/SimulateButton.png");
		m_IconStop = Texture2D::Upload("Resources/Icons/StopButton.png");

		m_EditorScene = CreateRef<Scene>();
		m_ActiveScene = m_EditorScene;

		auto commandLineArgs = Application::Get().GetSpecification().CommandLineArgs;
		if (commandLineArgs.Count > 1)
		{
			auto sceneFilePath = commandLineArgs[1];
			SceneSerializer serializer(m_ActiveScene);
			if (!serializer.Deserialize(sceneFilePath))
				HZ_CORE_ERROR("Could not deserialize scene from {0}", sceneFilePath);
		}

		m_EditorCamera = EditorCamera(m_CameraFOV, 1.778f, 0.1f, 100000.0f);
		Renderer2D::SetLineWidth(6.0f);

		m_EditorCamera.SetDistance(50.f);
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);

		m_MouseBody = CreateRef<Enyoo::RigidBody>();
		m_MouseBody->Disable = true;
		m_InteractionSpring = CreateRef<Enyoo::Spring>();
		m_InteractionSpring->SetRestLength(0.2);
		m_InteractionSpring->SetSpringConstant(9000.0);
		m_InteractionSpring->SetDampingValue(23.9);
		m_InteractionSpring->SetSecondBody(m_MouseBody.get());
		m_InteractionSpring->SetActive(false);
	}
	
	void EditorLayer::OnDetach()
	{
	}
	
	void EditorLayer::OnUpdate(Timestep ts)
	{
		HZ_PROFILE_FUNCTION();
		
		if (FrameBufferSpecification spec = m_FrameBuffer->GetSpecification();
			m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f &&
			(spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y))
		{
			m_FrameBuffer->Resize(static_cast<uint32_t>(m_ViewportSize.x), static_cast<uint32_t>(m_ViewportSize.y));
			m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
			m_ActiveScene->OnViewportResize(static_cast<uint32_t>(m_ViewportSize.x), static_cast<uint32_t>(m_ViewportSize.y));
		}
		
		//------------------Update----------------------------------
		m_EditorCamera.DisableRotation(m_DisableCameraRotation);
		m_EditorCamera.SetFOV(m_CameraFOV);

	
		//------------------Render----------------------------------
		Renderer2D::ResetStats();
		m_FrameBuffer->Bind();
		RenderCommand::SetClearColor({ 0.04f, 0.04f, 0.04f, 1 });
		RenderCommand::Clear();
		
		m_FrameBuffer->ClearAttachment(1, -1); //Clear ent ID to -1
		//------------------Scene----------------------------------
	

		switch (m_SceneState)
		{
			case SceneState::Edit:
			{
				m_EditorCamera.OnUpdate(ts);

				m_ActiveScene->OnUpdateEditor(ts, m_EditorCamera);
				break;
			}
			case SceneState::Simulate:
			{
				Timer timer;

				m_EditorCamera.OnUpdate(ts);

				m_ActiveScene->OnUpdateSimulation(ts, m_EditorCamera);
				//if (timer.ElapsedMilliseconds() > 200)
				//    OnSceneStop();
				break;
			}
			case SceneState::Play:
			{
				Timer timer;
				if (m_UseEditorCameraOnRuntime)
				{
					m_EditorCamera.OnUpdate(ts);
					m_ActiveScene->OnUpdateRuntime(ts, m_EditorCamera);
				}
				else
				{
					m_ActiveScene->OnUpdateRuntime(ts);
				}
				if (timer.ElapsedMilliseconds() > 200)
					OnSceneStop();
				break;
			}
		}

		//------------------Mouse Picking----------------------------
		auto [mx, my] = ImGui::GetMousePos();
		mx -= m_ViewportBounds[0].x;
		my -= m_ViewportBounds[0].y;
		glm::vec2 viewportSize = m_ViewportBounds[1] - m_ViewportBounds[0];
		my = viewportSize.y - my;
		int mouseX = static_cast<int>(mx);
		int mouseY = static_cast<int>(my);
		if (mouseX >= 0 && mouseY >= 0 && mouseX < static_cast<int>(viewportSize.x) && mouseY < static_cast<int>(viewportSize.y))
		{
			int pixelData = m_FrameBuffer->ReadPixel(1, mouseX, mouseY);
			m_HoveredEntity = pixelData == -1 ? Entity() : Entity(static_cast<entt::entity>(pixelData), m_ActiveScene.get());
		}

		DragHoveredEntity();

		OnOverlayRender();

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
				//ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen);
				ImGui::MenuItem("Padding", NULL, &opt_padding);
				ImGui::Separator();

				if (ImGui::MenuItem("New", "Ctrl+N"))
					NewScene();

				if (ImGui::MenuItem("Open...", "Ctrl+O"))
					OpenScene();

				if (ImGui::MenuItem("Save", "Ctrl+S"))
					SaveScene();

				if (ImGui::MenuItem("Save As", "Ctrl+Shift+S"))
					SaveSceneAs();

				if (ImGui::MenuItem("Quit", "Alt+F4"))
					Application::Get().CloseWindow();

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}


		ImGui::Begin("Statistics");

		std::string name = "None";
		if (m_HoveredEntity)
			name = m_HoveredEntity.GetComponent<TagComponent>().Tag;
		ImGui::Text("Hovered Entity: %s", name.c_str());

		auto stats = Renderer2D::GetStats();
		ImGui::Text("Renderer2D Stats: ");
		auto [mousex, mousey] = ImGui::GetMousePos();
		ImGui::Text("Mouse pos x: %i y: %i", static_cast<int>(mousex), static_cast<int>(mousey));
		ImGui::Text("Frametime: %f ms", Application::Get().GetLastFrameTime());
		ImGui::Text("FPS: %f", 1.f / Application::Get().GetLastFrameTime());
		ImGui::Text("Draw Calls: %d", stats.DrawCalls);
		ImGui::Text("Quads: %d", stats.QuadCount);
		ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
		ImGui::Text("Indices: %d", stats.GetTotalIndexCount());
		if(m_SceneState == SceneState::Simulate || m_SceneState == SceneState::Play)
			ImGui::Text("Total system energy: %f", m_ActiveScene->GetRigidBodySystem()->GetTotalSystemEnergy());
		ImGui::Separator();
 

		ImGui::End();

		ImGui::Begin("Settings");
		static float padding = 16.f;
		static float thumbnailSize = 128.f;
		static const uint16_t U16One = 1;
		static uint16_t steps = 1;
		static double dt = 0.01667;
		static double decDt = 0.001667;

		ImGui::SliderFloat("Thumbnail Size", &thumbnailSize, 16, 512);
		ImGui::SliderFloat("Padding", &padding, 0, 32);
		ImGui::DragFloat("Spring Constant Multiplier", &m_SpringConstMult, 0.1f, 0.01, 1200.0f);
		ImGui::InputScalar("Physics Steps", ImGuiDataType_U16, &steps, &U16One);
		ImGui::InputScalar("Delta Time", ImGuiDataType_Double, &dt, &decDt);
		ImGui::SameLine();
		if (ImGui::Button("Reset"))
			dt = 0.01667;
		ImGui::Checkbox("Show physics colliders", &m_ShowPhysicsColliders);
		ImGui::Checkbox("Use editor camera for runtime", &m_UseEditorCameraOnRuntime);
		ImGui::Checkbox("Disable camera rotation", &m_DisableCameraRotation);
		ImGui::Checkbox("Grab from centre", &m_DragFromCentre);
		if (ImGui::Button("Reset camera position"))
			m_EditorCamera.SetFocalPoint(glm::vec3(0.0f));
		ImGui::SliderFloat("FOV", &m_CameraFOV, 10.0f, 120.0f);
		ImGui::SameLine();
		if (ImGui::Button("x"))
			m_CameraFOV = 45.0f;
		m_ActiveScene->SetPhysicsDT(dt);
		m_ActiveScene->SetPhysicsSteps(steps);
		//ImGui::ShowDemoWindow(&dockspaceOpen);
		ImGui::End();

		m_SceneHierarchyPanel.OnImGuiRender();
		m_ContentBrowserPanel.OnImGuiRender(padding, thumbnailSize);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0,0 });
		ImGui::Begin("Viewport");
		auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
		auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
		auto viewportOffset = ImGui::GetWindowPos(); //includes tab bar
		m_ViewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };
		m_ViewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };

		m_ViewportFocused = ImGui::IsWindowFocused();
		m_ViewportHovered = ImGui::IsWindowHovered();                                            // either control key should prevent imgui blocking events
		Application::Get().GetImGuiLayer()->BlockEvents(!m_ViewportHovered && !m_ViewportFocused && !(Input::IsKeyPressed(HZ_KEY_LEFT_CONTROL) || Input::IsKeyPressed(HZ_KEY_RIGHT_CONTROL)));

		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
		ImVec2 viewportMousePos = ImGui::GetCursorScreenPos();

		m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };
		m_MouseViewportPos = { viewportMousePos.x , viewportMousePos.y };

		uint32_t textureID = m_FrameBuffer->GetColorAttachmentRendererID();
		ImGui::Image(reinterpret_cast<void*>(textureID), ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0,1 }, ImVec2{ 1,0 });

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				const wchar_t* path = static_cast<const wchar_t*>(payload->Data);
				OpenScene(std::filesystem::path(g_AssetPath) / path); //TODO: before opening another scene, prompt user for save changes
			}
			ImGui::EndDragDropTarget();
		}

		//Gizmos
		Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
		if (selectedEntity && m_GizmoType != -1)
		{
			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();

			float windowWidth = ImGui::GetWindowWidth();
			float windowHeight = ImGui::GetWindowHeight();
			ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);

			//Get Editor cam
			const glm::mat4& cameraProjection = m_EditorCamera.GetProjection();
			glm::mat4 cameraView = m_EditorCamera.GetViewMatrix();

			// Entity transfrom
			auto& tc = selectedEntity.GetComponent<TransformComponent>();
			glm::mat4 transform = tc.GetTransform();

			//Snapping
			bool snap = Input::IsKeyPressed(HZ_KEY_LEFT_CONTROL);
			float snapValue = 0.5f; //snap to 0.5m for translation
			//snap to 45 for rotation
			if (m_GizmoType == ImGuizmo::OPERATION::ROTATE)
				snapValue = 45.0f;

			float snapValues[3] = { snapValue, snapValue, snapValue };

			ImGuizmo:Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
			static_cast<ImGuizmo::OPERATION>(m_GizmoType), ImGuizmo::LOCAL, glm::value_ptr(transform),
			nullptr, snap ? snapValues : nullptr);

			if (ImGuizmo::IsUsing())
			{
				glm::vec3 translation, rotation, scale;
				Math::DecomposeTransform(transform, translation, rotation, scale);

				glm::vec3 deltaRotation = rotation - tc.Rotation;
				tc.Translation = translation;
				tc.Rotation += deltaRotation;
				tc.Scale = scale;
			}
		}

		ImGui::End();
		ImGui::PopStyleVar();

		// UI Tool bar
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 2));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		auto& colors = ImGui::GetStyle().Colors;
		const auto& buttonHovered = colors[ImGuiCol_ButtonHovered];
		const auto& buttonActive = colors[ImGuiCol_ButtonActive];
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(buttonHovered.x, buttonHovered.y, buttonHovered.z, 0.5f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(buttonActive.x, buttonActive.y, buttonActive.z, 0.5f));

		ImGui::Begin("##toolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar || ImGuiWindowFlags_NoScrollWithMouse);

		bool toolbarEnabled = static_cast<bool>(m_ActiveScene);

		ImVec4 tintColor = ImVec4(1, 1, 1, 1);
		if (!toolbarEnabled)
			tintColor.w = 0.5f;

		float size = ImGui::GetWindowHeight() - 4.0f;
		
		Ref<Texture2D> icon1 = (m_SceneState == SceneState::Edit || m_SceneState == SceneState::Simulate) ? m_IconPlay : m_IconStop;
		ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (size * 0.5f));
		if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(icon1->GetRendererID()), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor) && toolbarEnabled)
		{
			if (m_SceneState == SceneState::Edit || m_SceneState == SceneState::Simulate)
				OnScenePlay();
			else if (m_SceneState == SceneState::Play)
				OnSceneStop();
		}
		
		ImGui::SameLine();
		
		Ref<Texture2D> icon2 = m_SceneState == SceneState::Edit || m_SceneState == SceneState::Play ? m_IconSimulate : m_IconStop;
		if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(icon2->GetRendererID()), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0))
		{
			if (m_SceneState == SceneState::Edit || m_SceneState == SceneState::Play)
				OnSceneSimulate();
			else if (m_SceneState == SceneState::Simulate)
				OnSceneStop();
		}

		DeleteDeffered();

		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(3);
		ImGui::End(); //UI tool bar end

		//View port end
		ImGui::End();
	}
  
	void EditorLayer::OnOverlayRender()
	{
		if (m_SceneState == SceneState::Play && !m_UseEditorCameraOnRuntime)
		{
			Entity camera = m_ActiveScene->GetPrimaryCameraEntity();
			if (!camera)
				return;

			Renderer2D::BeginScene(camera.GetComponent<CameraComponent>().Camera, camera.GetComponent<TransformComponent>().GetTransform());
		}
		else
		{
			Renderer2D::BeginScene(m_EditorCamera);
		}

		if (m_ShowPhysicsColliders)
		{
			// Box Colliders
			{
				auto view = m_ActiveScene->GetAllEntitiesWith<TransformComponent, BoxCollider2DComponent>();
				for (auto entity : view)
				{
					auto [tc, bc2d] = view.get<TransformComponent, BoxCollider2DComponent>(entity);

					glm::vec3 translation = tc.Translation + glm::vec3(bc2d.Offset, 0.001f);
					glm::vec3 scale = tc.Scale * glm::vec3(bc2d.Size * 2.0f, 1.0f);

					glm::mat4 transform = glm::translate(glm::mat4(1.0f), tc.Translation)
						* glm::rotate(glm::mat4(1.0f), tc.Rotation.z, glm::vec3(0.0f, 0.0f, 1.0f))
						* glm::translate(glm::mat4(1.0f), glm::vec3(bc2d.Offset, 0.001f))
						* glm::scale(glm::mat4(1.0f), scale);

					Renderer2D::DrawRect(transform, glm::vec4(0, 1, 0, 1));
				}
			}

			// Circle Colliders
			{
				auto view = m_ActiveScene->GetAllEntitiesWith<TransformComponent, CircleCollider2DComponent>();
				for (auto entity : view)
				{
					auto [tc, cc2d] = view.get<TransformComponent, CircleCollider2DComponent>(entity);

					glm::vec3 translation = tc.Translation + glm::vec3(cc2d.Offset, 0.001f);
					glm::vec3 scale = tc.Scale * glm::vec3(cc2d.Radius * 2.0f);

					glm::mat4 transform = glm::translate(glm::mat4(1.0f), tc.Translation)
						* glm::rotate(glm::mat4(1.0f), tc.Rotation.z, glm::vec3(0.0f, 0.0f, 1.0f))
						* glm::translate(glm::mat4(1.0f), glm::vec3(cc2d.Offset, 0.001f))
						* glm::scale(glm::mat4(1.0f), scale);
					
					Renderer2D::DrawCircle(transform, glm::vec4(0, 1, 0, 1), 0.08f);
				}
			}
		}

		if (Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity())
		{
			// highlight selected entity
			const auto& transform = selectedEntity.GetComponent<TransformComponent>();
			glm::mat4 selectedTransform = 
				glm::translate(glm::mat4(1.0f), { transform.Translation.x, transform.Translation.y, transform.Translation.z + 0.001f })
				* glm::rotate(glm::mat4(1.0f), transform.Rotation.z, glm::vec3(0.0f, 0.0f, 1.0f))
				* glm::scale(glm::mat4(1.0f), { transform.Scale.x, transform.Scale.y, transform.Scale.z });
			Renderer2D::DrawRect(selectedTransform, glm::vec4(0.2f, 0.4f, 1.0f, 1.0f));	

			// draw snap points
			if (m_SceneState == SceneState::Edit && selectedEntity.HasComponent<LinkPointsComponent>())
			{
				auto view = m_ActiveScene->m_Registry.view<TransformComponent, LinkPointsComponent>();
				for (auto e : view)
				{
					Entity entity = { e, m_ActiveScene.get() };
					auto& tc = entity.GetComponent<TransformComponent>();

					auto range = m_ActiveScene->GetLinkPoints(entity.GetUUID());

					for (auto it = range.first; it != range.second; it++)
					{
						glm::vec4 drawColor = { 0.2f, 0.2f, 0.9f, 1.0f };
						glm::vec2 world = Enyoo::Utilities::LocalToWorld(it->second, tc.Rotation, tc.Translation);
						glm::mat4 pointTransform = glm::translate(glm::mat4(1.f), {world.x, world.y, 0.3f})
							* glm::scale(glm::mat4(1.f), glm::vec3{0.3f});

						if (selectedEntity == entity)
							drawColor = { 1.0f, 0.2f, 0.1f, 1.0f };

						Renderer2D::DrawCircle(pointTransform, drawColor);
					}
				}
			}
		}

		

		Renderer2D::EndScene();
	}

	void EditorLayer::OnEvent(Event& event)
	{
		if(m_ViewportHovered)
			m_EditorCamera.OnEvent(event);

		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<KeyPressedEvent>(HZ_BIND_EVENT_FN(EditorLayer::OnKeyPressed)); 
		dispatcher.Dispatch<MouseButtonPressedEvent>(HZ_BIND_EVENT_FN(EditorLayer::OnMouseClick));
		dispatcher.Dispatch<MouseButtonReleasedEvent>(HZ_BIND_EVENT_FN(EditorLayer::OnMouseRelease));
	}

	bool EditorLayer::OnKeyPressed(KeyPressedEvent& event)
	{
		if (event.IsRepeat())
			return false;

		bool control = Input::IsKeyPressed(HZ_KEY_LEFT_CONTROL) || Input::IsKeyPressed(HZ_KEY_RIGHT_CONTROL);
		bool shift = Input::IsKeyPressed(HZ_KEY_LEFT_SHIFT) || Input::IsKeyPressed(HZ_KEY_RIGHT_SHIFT);
		bool alt = Input::IsKeyPressed(HZ_KEY_LEFT_ALT) || Input::IsKeyPressed(HZ_KEY_RIGHT_ALT);

		switch (event.GetKeyCode())
		{
			//file shortcuts
			case HZ_KEY_N:
			{
				if (control)
					NewScene();
				break;
			}
			case HZ_KEY_O:
			{
				if (control)
					OpenScene();
				break;
			}
			case HZ_KEY_S:
			{
				if (control)
				{
					if (shift)
						SaveSceneAs();
					else
						SaveScene();
				}

				break;
			}
			case HZ_KEY_D:
			{
				if (control)
					OnDuplicateEntity();
				break;
			}
			case HZ_KEY_F4:
			{
				if (alt)
					Application::Get().CloseWindow();
				break;
			}
			//gizmo shortcuts
			case HZ_KEY_Q:
				if(!ImGuizmo::IsUsing())
					m_GizmoType = -1;
				break;
			case HZ_KEY_W: 
				if (!ImGuizmo::IsUsing())
					m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
				break;
			case HZ_KEY_E:
				if (!ImGuizmo::IsUsing())
					m_GizmoType = ImGuizmo::OPERATION::ROTATE;
				break;
			case HZ_KEY_R:
				if (!ImGuizmo::IsUsing())
					m_GizmoType = ImGuizmo::OPERATION::SCALE;
				break;
			//settings
			case HZ_KEY_X:
				if (alt)
					m_ShowPhysicsColliders ^= true;
				break;
		}
	}

	bool EditorLayer::OnMouseClick(MouseButtonEvent& event)
	{
		if (event.GetMouseButton() == HZ_MOUSE_BUTTON_LEFT)
		{
			if (SetDependency(m_SceneHierarchyPanel.GetSelectedEntity(), m_HoveredEntity))
				return true;

			if (m_ViewportHovered && !ImGuizmo::IsOver() && !Input::IsMouseButtonPressed(HZ_MOUSE_BUTTON_5))           
				m_SceneHierarchyPanel.SetSelectionContext(m_HoveredEntity);

			if (m_HoveredEntity && m_HoveredEntity.HasComponent<RigidBodyComponent>() && 
				(m_SceneState == SceneState::Play || m_SceneState == SceneState::Simulate))
			{
				m_DragEntity = m_HoveredEntity;

				auto* body = m_DragEntity.GetComponent<RigidBodyComponent>().RuntimeBody.get();
				auto [x, y] = ImGui::GetMousePos();
				glm::dvec2 local = m_DragFromCentre ? glm::dvec2(0.0) : body->WorldToLocal(GetWorldPosFromMouse({ x, y })); 
				m_InteractionSpring->SetFirstBody(body);
				m_InteractionSpring->SetFirstPosition(local); 
				m_InteractionSpring->TorqueLock(m_DragFromCentre);
			}			
		}

		return false;
	}

	bool EditorLayer::OnMouseRelease(MouseButtonEvent& event)
	{
		if (m_SceneState == SceneState::Edit)
		   ObjectSnapping(m_SceneHierarchyPanel.GetSelectedEntity());

		return false;
	}

	void EditorLayer::NewScene()
	{
		m_EditorScene = CreateRef<Scene>();
		m_EditorScene->OnViewportResize(static_cast<uint32_t>(m_ViewportSize.x), static_cast<uint32_t>(m_ViewportSize.y));
		m_SceneHierarchyPanel.SetContext(m_EditorScene);
		m_ActiveScene = m_EditorScene;

		m_EditorScenePath = std::filesystem::path();
	}

	void EditorLayer::OpenScene()
	{                                                       //(allfiles ("*")) - the actual filter
		std::string filepath = FileDialogs::OpenFile("Hazel Scene (*.hazel)\0*.hazel\0");
		if (!filepath.empty())
		{
			OpenScene(filepath);
		}
	}

	void EditorLayer::OpenScene(const std::filesystem::path& filepath)
	{
		Timer timer;
		if (m_SceneState != SceneState::Edit)
			OnSceneStop();

		if (filepath.extension().string() != ".hazel")
		{
			HZ_CORE_WARN("Could not load {0} - not a scene file", filepath.filename().string());
			return;
		}

		Ref<Scene> newScene = CreateRef<Scene>();
		SceneSerializer serializer(newScene);
		if (serializer.Deserialize(filepath)) 
		{
			m_EditorScene = newScene;
			m_EditorScene->OnViewportResize(static_cast<uint32_t>(m_ViewportSize.x), static_cast<uint32_t>(m_ViewportSize.y));
			m_SceneHierarchyPanel.SetContext(m_EditorScene);

			m_ActiveScene = m_EditorScene;
			m_EditorScenePath = filepath;
		}

		HZ_CORE_TRACE("Scene took {0} ms ", timer.ElapsedMilliseconds());
	}

	void EditorLayer::SaveScene()
	{
		if (m_SceneState != SceneState::Edit)
			return;

		if (!m_EditorScenePath.empty())
			SerializeScene(m_ActiveScene, m_EditorScenePath);
		else
			SaveSceneAs();
	}

	void EditorLayer::SaveSceneAs()
	{                                                               
		std::string filepath = FileDialogs::SaveFile("Hazel Scene (*.hazel)\0*.hazel\0");
		if (!filepath.empty())
		{
			SerializeScene(m_ActiveScene, filepath);
			m_EditorScenePath = filepath;
		}
	}

	void EditorLayer::SerializeScene(Ref<Scene> scene, const std::filesystem::path& path)
	{
		SceneSerializer serializer(scene);
		serializer.Serialize(path.string());
	}

	void EditorLayer::OnScenePlay()
	{
		if (m_SceneState == SceneState::Simulate)
			OnSceneStop();

		m_SceneState = SceneState::Play;
		m_GizmoType = -1;

		m_ActiveScene = Scene::Copy(m_EditorScene);
		m_ActiveScene->OnRuntimeStart();

		PersistSelectionContext();
	}

	void EditorLayer::OnSceneSimulate()
	{
		if (m_SceneState == SceneState::Play)
			OnSceneStop();

		m_SceneState = SceneState::Simulate;
		m_GizmoType = -1;

		m_ActiveScene = Scene::Copy(m_EditorScene);
		m_ActiveScene->OnSimulationStart();
		m_ActiveScene->GetRigidBodySystem()->AddForceGen(m_InteractionSpring);
		m_MouseBody->Index = -1;

		PersistSelectionContext();
	}

	void EditorLayer::OnSceneStop()
	{
		HZ_CORE_ASSERT(m_SceneState == SceneState::Play || m_SceneState == SceneState::Simulate, "Scene state not set");

		if (m_SceneState == SceneState::Play)
			m_ActiveScene->OnRuntimeStop();
		else if (m_SceneState == SceneState::Simulate)
			m_ActiveScene->OnSimulationStop();

		m_SceneState = SceneState::Edit;
		m_ActiveScene = m_EditorScene;
		m_HoveredEntity = {};

		PersistSelectionContext();
	}

	void EditorLayer::DragHoveredEntity()
	{
		if (m_SceneState != SceneState::Simulate && m_SceneState != SceneState::Play)
			return;

		auto [x, y] = ImGui::GetMousePos();
		glm::vec2 mouseWorld = GetWorldPosFromMouse({ x, y });
		m_MouseBody->Position = { mouseWorld.x, -mouseWorld.y };
		if (!m_DragEntity)
			return;
		
		if (Input::IsMouseButtonPressed(HZ_MOUSE_BUTTON_LEFT))
		{
			Renderer2D::BeginScene(m_EditorCamera);
			auto* body = m_DragEntity.GetComponent<RigidBodyComponent>().RuntimeBody.get();

			m_InteractionSpring->SetActive(true);
			m_InteractionSpring->SetSpringConstant(body->Mass * static_cast<double>(m_SpringConstMult));

			glm::mat4 transform = glm::translate(glm::mat4(1.f), { mouseWorld, 0.2f })
				* glm::scale(glm::mat4(1.f), glm::vec3(0.5));
			Renderer2D::DrawCircle(transform, { 1.0f, 0.2f, 0.3f, 1.0f });
			Renderer2D::EndScene();
		}
		else
		{
			m_DragEntity = Entity(); 
			m_InteractionSpring->SetActive(false);
		}	
	}

	void EditorLayer::OnDuplicateEntity()
	{
		if (m_SceneState != SceneState::Edit)
			return;

		Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();

		if (selectedEntity)
			m_SceneHierarchyPanel.SetSelectionContext(m_EditorScene->DuplicateEntity(selectedEntity));
	}

	bool EditorLayer::SetDependency(Entity focus, Entity target)
	{
		if (!m_SceneHierarchyPanel.DependencyCheck())
			return false;

		if (!target.HasComponent<RigidBodyComponent>())
		{
			m_SceneHierarchyPanel.CheckTerminate();
			return false;
		}

		UUID targetuuid = target.GetComponent<IDComponent>().ID;
		auto& fgc = focus.GetComponent<ForceGeneratorComponent>();
		fgc.targetID = targetuuid;

		m_SceneHierarchyPanel.CheckTerminate();
		return true;
	}

	void EditorLayer::DeleteDeffered()
	{
		Entity deff = m_SceneHierarchyPanel.GetDefferedEntity();

		if (!deff)
			return;

		m_ActiveScene = Scene::CopyExclusive(m_EditorScene, deff);
		m_ActiveScene->RemoveLinkPoints(deff.GetUUID());
		m_EditorScene = m_ActiveScene;
		m_SceneHierarchyPanel.ResetDeffered();
		m_SceneHierarchyPanel.SetContext(m_EditorScene);
		m_SceneHierarchyPanel.CheckTerminate();
		m_HoveredEntity = {};
	}

	void EditorLayer::PersistSelectionContext()
	{
		if (m_SceneHierarchyPanel.GetSelectedEntity() != Entity())
		{
			auto view = m_ActiveScene->GetAllEntitiesWith<IDComponent>();
			auto newSelection = view.find(static_cast<entt::entity>(m_SceneHierarchyPanel.GetSelectedEntity()));
			m_SceneHierarchyPanel.SetContext(m_ActiveScene);
			m_SceneHierarchyPanel.SetSelectionContext(Entity(*newSelection, m_ActiveScene.get()));
			return;
		}

		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
	}

	void EditorLayer::ObjectSnapping(Entity entity)
	{
		if (!entity)
			return;

		if (!entity.HasComponent<LinkPointsComponent>())
			return;

		auto view = m_ActiveScene->m_Registry.view<LinkPointsComponent, TransformComponent>();
		auto& selectedTransform = entity.GetComponent<TransformComponent>();
		auto range = m_ActiveScene->GetLinkPoints(entity.GetUUID());
		for (auto e : view)
		{
			Entity otherEntity = { e, m_ActiveScene.get() };

			if (otherEntity == entity)
				continue;

			auto otherRange = m_ActiveScene->GetLinkPoints(otherEntity.GetUUID());
			auto& otherTransform = view.get<TransformComponent>(e);

			for (auto it = otherRange.first; it != otherRange.second; it++)
			{
				//convert otherPoint to world with e's transform
				glm::vec2 otherWorld = Enyoo::Utilities::LocalToWorld(it->second, otherTransform.Rotation, otherTransform.Translation);

				for (auto iter = range.first; iter != range.second; iter++)
				{
					//convert point to world with selected's transform
					glm::vec2 world = Enyoo::Utilities::LocalToWorld(iter->second, selectedTransform.Rotation, selectedTransform.Translation);

					if (std::fabs(otherWorld.x - world.x) < 0.6 && std::fabs(otherWorld.y - world.y) < 0.6)
					{
						//update selected's position to from snap point
						selectedTransform.Translation.x = otherWorld.x + (selectedTransform.Translation.x - world.x);
						selectedTransform.Translation.y = otherWorld.y + (selectedTransform.Translation.y - world.y);
					}
				}
			}
		}	
	}

	glm::vec2 EditorLayer::GetWorldPosFromMouse(glm::vec2 mousePos)
	{
		//this method only works when there is no rotation or FOV changes from the editor camera (both need to be disabled to work)

		glm::vec2 ndc = (mousePos - m_MouseViewportPos) / m_ViewportSize;
		ndc.x = 2.0f * ndc.x - 1.0f;
		ndc.y = 1.0f - (2.0f * ndc.y);
		float cameraWidth = m_EditorCamera.GetBounds().GetWidth();
		float cameraHeight = m_EditorCamera.GetBounds().GetHeight();
		glm::vec3 focalPoint = m_EditorCamera.GetFocalPoint();
		glm::vec2 worldPos = { ndc.x * cameraWidth / 2.0f, ndc.y * cameraHeight / 2.0f };
		worldPos.x += focalPoint.x;
		worldPos.y += focalPoint.y;

		return worldPos;
	}
}
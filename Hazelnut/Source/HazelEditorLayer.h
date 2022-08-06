#pragma once

#include "Hazel.h"
#include "Hazel/Core/Layer.h"
#include "Panels/SceneHierarchyPanel.h"
#include "Panels/ContentViewPanel.h"

namespace Hazel
{
	class EditorLayer : public Layer
	{
	public:
		EditorLayer();
		virtual ~EditorLayer() = default;
	
		void OnAttach() override;
		void OnDetach() override;
		void OnUpdate(Timestep ts) override;
		void OnImGuiRender() override;
		void OnEvent(Event& event) override;
	
	private:
		bool OnKeyPressed(KeyPressedEvent& event);
		bool OnMouseClick(MouseButtonEvent& event);

		void OnOverlayRender();

		void NewScene();
		void OpenScene();
		void OpenScene(const std::filesystem::path& path);
		void SaveScene();
		void SaveSceneAs();

		void SerializeScene(Ref<Scene> scene, const std::filesystem::path& path);

		void OnScenePlay();
		void OnSceneSimulate();
		void OnSceneStop();

		void OnDuplicateEntity();

		void UIToolbar();
	private:
		Ref<FrameBuffer> m_FrameBuffer;
		Ref<Scene> m_ActiveScene;
		Ref<Scene> m_EditorScene, m_RuntimeScene;
		Ref<Texture2D> m_IconPlay, m_IconSimulate, m_IconStop;
		Entity m_HoveredEntity;
		Entity m_TestEnt;

		EditorCamera m_EditorCamera;
	
		glm::vec2 m_ViewportSize = {0.0f, 0.0f};
		glm::vec2 m_ViewportBounds[2];
		bool m_ViewportFocused = false;
		bool m_ViewportHovered = false;
	
		std::filesystem::path m_EditorScenePath;
		int m_GizmoType = -1;
		bool m_ShowPhysicsColliders = false;
		//panels
		SceneHierarchyPanel m_SceneHierarchyPanel;
		ContentViewPanel m_ContentBrowserPanel;

		enum class SceneState
		{
			Edit = 0,
			Play,
			Simulate,
			Stop
		};

		SceneState m_SceneState = SceneState::Edit;
	};
}


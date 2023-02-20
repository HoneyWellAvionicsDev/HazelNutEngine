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
		void OnOverlayRender();

		bool OnKeyPressed(KeyPressedEvent& event);
		bool OnMouseClick(MouseButtonEvent& event);
		bool OnMouseRelease(MouseButtonEvent& event);

		void NewScene();
		void OpenScene();
		void OpenScene(const std::filesystem::path& path);
		void SaveScene();
		void SaveSceneAs();

		void SerializeScene(Ref<Scene> scene, const std::filesystem::path& path);

		void OnScenePlay();
		void OnSceneSimulate();
		void OnSceneStop();

		void DragHoveredEntity();
		void OnDuplicateEntity();
		void PersistSelectionContext();
		void ObjectSnapping(Entity entity);
		glm::vec2 GetWorldPosFromMouse(glm::vec2 mousePos);
	private:
		EditorCamera m_EditorCamera;

		Ref<FrameBuffer> m_FrameBuffer;
		Ref<Scene> m_ActiveScene;
		Ref<Scene> m_EditorScene, m_RuntimeScene;
		Ref<Texture2D> m_IconPlay, m_IconSimulate, m_IconStop;
		Ref<Enyoo::Spring> m_InteractionSpring = nullptr;
		Ref<Enyoo::RigidBody> m_MouseBody = nullptr;
		Entity m_HoveredEntity;
		Entity m_DragEntity;

		glm::vec2 m_ViewportSize = { 0.0f, 0.0f };
		glm::vec2 m_MouseViewportPos = { 0.0f, 0.0f };
		glm::vec2 m_ViewportBounds[2];
		bool m_ViewportFocused = false;
		bool m_ViewportHovered = false;
	
		std::filesystem::path m_EditorScenePath;
		int m_GizmoType = -1;
		bool m_ShowPhysicsColliders = false;
		bool m_UseEditorCameraOnRuntime = false;
		bool m_DisableCameraRotation = false;
		float m_CameraFOV = 45.0f;
		float m_Gravity[2] = { 0.f, -9.81f };
		int m_VeloctiyIterations = 6;
		int m_PositionIterations = 2;
		//panels
		SceneHierarchyPanel m_SceneHierarchyPanel;
		ContentViewPanel m_ContentBrowserPanel;

		enum class SceneState //TODO: scene state should really be stored in scene and accessed here
		{
			Edit = 0,
			Play,
			Simulate,
			Stop
		};

		SceneState m_SceneState = SceneState::Edit;
	};
}


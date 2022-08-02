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

		void NewScene();
		void OpenScene();
		void OpenScene(const std::filesystem::path& path);
		void SaveSceneAs();
	private:
		Hazel::OrthographicCameraController m_CameraController;
	
		Ref<FrameBuffer> m_FrameBuffer;
		Ref<Scene> m_Scene;
		Entity m_HoveredEntity;

		EditorCamera m_EditorCamera;
	
		glm::vec2 m_ViewportSize = {0.0f, 0.0f};
		glm::vec2 m_ViewportBounds[2];
		bool m_ViewportFocused = false;
		bool m_ViewportHovered = false;
	
		std::unordered_map<char, Ref<SubTexture2D>> s_TextureMap;
		int m_GizmoType = -1;
		//panels
		SceneHierarchyPanel m_SceneHierarchyPanel;
		ContentViewPanel m_ContentBrowserPanel;
	};
}


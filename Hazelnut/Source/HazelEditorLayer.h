#pragma once

#include "Hazel.h"
#include "Hazel/Core/Layer.h"
#include "Panels/SceneHierarchyPanel.h"

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
		void SaveSceneAs();
	private:
		Hazel::OrthographicCameraController m_CameraController;
	
		//temp
		Ref<FrameBuffer> m_FrameBuffer;
		Ref<VertexArray> m_SquareVA;
		Ref<Shader> m_FlatColorShader;
		Ref<Texture2D> m_Texture;
		Ref<Texture2D> m_Texture2;
		Ref<Texture2D> m_SpriteSheet;
		Ref<SubTexture2D> m_FullHeart;
		Ref<Scene> m_Scene;

		Entity m_SquareEntity;
		Entity m_CameraEntity;
		Entity m_CameraEntity2;
		Entity m_HoveredEntity;

		bool m_PrimaryCamera = true;
		EditorCamera m_EditorCamera;
	
		glm::vec4 m_SquareColor = { 0.2f, 0.3f, 0.8f, 1.0f };
		glm::vec2 m_ViewportSize = {0.0f, 0.0f};
		glm::vec2 m_ViewportBounds[2];
		bool m_ViewportFocused = false;
		bool m_ViewportHovered = false;
		float m_RotationalSpeed = 60.0f;
	
		std::unordered_map<char, Ref<SubTexture2D>> s_TextureMap;
		int m_GizmoType = -1;
		//panels
		SceneHierarchyPanel m_SceneHierarchyPanel;
	};
}


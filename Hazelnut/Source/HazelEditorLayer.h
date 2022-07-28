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
		bool m_PrimaryCamera = true;
	
		glm::vec4 m_SquareColor = { 0.2f, 0.3f, 0.8f, 1.f };
		glm::vec2 m_ViewportSize = {0.f, 0.f};
		bool m_ViewportFocused = false;
		bool m_ViewportHovered = false;
		float m_RotationalSpeed = 60.f;
	
		std::unordered_map<char, Ref<SubTexture2D>> s_TextureMap;


		//panels
		SceneHierarchyPanel m_SceneHierarchyPanel;
	};
}


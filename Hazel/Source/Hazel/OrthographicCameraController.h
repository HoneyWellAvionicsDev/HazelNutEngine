#pragma once

#include "Hazel/Renderer/OrthographicCamera.h"
#include "Hazel/Core/TimeStep.h"
#include "Hazel/Events/ApplicationEvent.h"
#include "Hazel/Events/MouseEvent.h"

namespace Hazel
{
	class OrthographicCameraController
	{
	public:
		OrthographicCameraController(float aspectRatio, bool rotation = false);

		void OnUpdate(Timestep ts);
		void OnEvent(Event& e);

		OrthographicCamera& GetCamera() { return m_Camera; }
		const OrthographicCamera& GetCamera() const { return m_Camera; }
	private:
		bool OnMouseScrolled(MouseScrolledEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);

		float m_AspectRatio;
		float m_ZoomLevel = 1.f;
		OrthographicCamera m_Camera;


		glm::vec3 m_CameraPosition = { 0.f, 0.f, 0.f };
		float m_CameraTranslationSpeed = 5.5f;

		bool m_Rotation;
		float m_CameraRotationAngle = 0;
		float m_CameraRotationSpeed = 45.f;
	};
}


#pragma once

#include "Camera.h"
#include "Hazel/Core/TimeStep.h"
#include "Hazel/Events/Event.h"
#include "Hazel/Events/MouseEvent.h"

#include <glm/glm.hpp>


namespace Hazel
{
	class EditorCamera : public Camera
	{
	public:
		EditorCamera() = default;
		EditorCamera(float fov, float aspectRatio, float nearClip, float farClip);

		void OnUpdate(Timestep ts);
		void OnEvent(Event& event);

		void SetDistance(float distance) { m_Distance = distance; }
		void SetPosition(const glm::vec3& position) { m_Position = position; }
		void SetFocalPoint(const glm::vec3& focalPoint) { m_FocalPoint = focalPoint; }
		void SetFOV(float fov) { m_FOV = fov; UpdateProjection(); }
		void DisableRotation(bool disable) { m_DisableRotation = disable; }

		inline void SetViewportSize(float width, float height) { m_ViewportWidth = width; m_ViewportHeight = height; UpdateProjection(); }

		const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		glm::mat4 GetViewProjection() const { return m_Projection * m_ViewMatrix; }

		const glm::vec3& GetPosition() const { return m_Position; }
		CameraBounds GetBounds() const { return m_Bounds; }
		glm::vec3 GetUpDirection() const;
		glm::vec3 GetRightDirection() const;
		glm::vec3 GetForwardDirection() const;
		glm::quat GetOrientation() const;

		glm::vec3 GetFocalPoint() const { return m_FocalPoint; }
		float GetDistance() const { return m_Distance; }
		float GetPitch() const { return m_Pitch; }
		float GetYaw() const { return m_Yaw; }
	private:
		void UpdateProjection();
		void UpdateView();
		glm::vec3 CalculatePosition() const;

		bool OnMouseScroll(MouseScrolledEvent& event);

		void MousePan(const glm::vec2& delta);
		void MouseRotate(const glm::vec2& delta);
		void MouseZoom(float delta);

		std::pair<float, float> PanSpeed() const;
		float RotationSpeed() const;
		float ZoomSpeed() const;
	private:
		float m_FOV = 45.0f, m_AspectRatio = 1.778f, m_NearClip = 0.1f, m_FarClip = 1000.0f;

		glm::mat4 m_ViewMatrix;
		glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
		glm::vec3 m_FocalPoint = { 0.0f, 0.0f, 0.0f };

		glm::vec2 m_InitialMousePosition = { 0.0f, 0.0f };

		float m_Distance = 10.0f;
		float m_Pitch = 0.0f;
		float m_Yaw = 0.0f;
		bool m_DisableRotation = false;

		float m_ViewportWidth = 1280, m_ViewportHeight = 720;
		CameraBounds m_Bounds;
	};
}


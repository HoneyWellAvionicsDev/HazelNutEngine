#pragma once

#include "Hazel/Renderer/Camera.h"

namespace Hazel
{
	class SceneCamera : public Camera
	{
	public:
		enum class ProjectionType { Perspective = 0, Orthographic = 1 };

		SceneCamera();
		virtual ~SceneCamera() = default;

		float GetOrthographicNearClip() const { return m_OrthographicNear; }
		float GetOrthographicFarClip() const { return m_OrthographicFar; }
		float GetOrthographicSize() const { return m_OrthographicSize; }
		
		float GetPerspectiveVerticalFOV() const { return glm::degrees(m_PerspectiveVerticalFOV); }
		float GetPerspectiveHorizontalFOV() const { return glm::degrees(m_PerspectiveHorizontalFOV); }
		float GetPerspectiveNearClip() const { return m_PerspectiveNear; }
		float GetPerspectiveFarClip() const { return m_PerspectiveFar; }

		ProjectionType GetProjectionType() const { return m_ProjectionType; }

		void SetViewportSize(uint32_t width, uint32_t height);

		void SetOrthographic(float size, float nearClip, float farClip);
		void SetOrthographicNearClip(float nearClip) { m_OrthographicNear = nearClip; RecalculateProjection(); }
		void SetOrthographicFarClip(float farClip) { m_OrthographicFar = farClip; RecalculateProjection(); }
		void SetOrthographicSize(float orthographicSize) { m_OrthographicSize = orthographicSize; RecalculateProjection(); }

		void SetPerspective(float vertFOV, float horzFOV, float nearClip, float farClip);
		void SetPerspectiveVerticalFOV(float perspectiveVertFov) { m_PerspectiveVerticalFOV = glm::radians(perspectiveVertFov); RecalculateProjection(); }
		void SetPerspectiveHorizontalFOV(float perspectiveHorzFov) { m_PerspectiveHorizontalFOV = glm::radians(perspectiveHorzFov); RecalculateProjection(); }
		void SetPerspectiveNearClip(float nearClip) { m_PerspectiveNear = nearClip; }
		void SetPerspectiveFarClip(float farClip) { m_PerspectiveFar = farClip; }

		void SetProjectionType(ProjectionType type) { m_ProjectionType = type; }
		
	private:
		void RecalculateProjection();

		float m_OrthographicSize = 10.f;
		float m_OrthographicNear = -1.f;
		float m_OrthographicFar =   1.f;

		float m_PerspectiveVerticalFOV = glm::radians(45.f);
		float m_PerspectiveHorizontalFOV = glm::radians(90.f);
		float m_PerspectiveNear = 0.01f;
		float m_PerspectiveFar = 10000.f;

		float m_AspectRatio = 0.f;
		ProjectionType m_ProjectionType = ProjectionType::Orthographic;

	};
}


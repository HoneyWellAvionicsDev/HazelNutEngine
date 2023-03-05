#pragma once

#include <glm/glm.hpp>

namespace Jbonk
{
	class OrthographicCamera
	{
	public:
		OrthographicCamera(float left, float right, float bottom, float top);

		void SetProjection(float left, float right, float bottom, float top);

		const glm::vec3& GetPosition() const { return m_Position; }
		float GetRotation() const { return m_ZRotation; }

		void SetPosition(const glm::vec3& position) 
		{ 
			m_Position = position; 
			RecalculateViewMatrix();
		}
		void SetRotation(const float rotation) 
		{
			m_ZRotation = rotation; 
			RecalculateViewMatrix();
		}


		const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
		const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		const glm::mat4& GetViewProjectionMatrix() const { return m_ViewProjectionMatrix; }
	private:
		void RecalculateViewMatrix();                                //this will be called AFTER we set new pos and rotation values

		glm::mat4 m_ProjectionMatrix;
		glm::mat4 m_ViewMatrix;										 //inverse of transformation matrix of the camera
		glm::mat4 m_ViewProjectionMatrix;

		glm::vec3 m_Position {0.f, 0.f, 0.f};
		float m_ZRotation = 0.f;                                     //rotation along Z axis
	};
}


#pragma once

#include <glm/glm.hpp>

namespace Hazel
{
	class Camera
	{
	public:
		Camera() = default;
		Camera(const glm::mat4& projection)
			: m_Projection(projection){}
		virtual ~Camera() = default;

		const glm::mat4& GetProjection() const { return m_Projection; }

		//todo void set prespective
	protected:
		glm::mat4 m_Projection = glm::mat4(1.f);
	};
}


#pragma once

#include <glm/glm.hpp>

namespace Jbonk
{
	struct CameraBounds
	{
		float Left, Right;
		float Bottom, Top;

		float GetWidth() { return Right - Left; }
		float GetHeight() { return Top - Bottom; }
	};

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


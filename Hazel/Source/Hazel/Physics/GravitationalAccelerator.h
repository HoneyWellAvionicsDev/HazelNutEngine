#pragma once

#include "ForceGenerator.h"

namespace Enyoo
{
	class GravitationalAccelerator : public ForceGenerator
	{
	public:
		GravitationalAccelerator();

		virtual void ApplyForce(SystemState& state) override;

		void SetGravity(glm::dvec2 gravity) { m_g = gravity; }
	private:
		glm::dvec2 m_g;
	};
}
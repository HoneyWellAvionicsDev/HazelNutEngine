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
		glm::dvec2 GetGravity() const { return m_g; }
	private:
		glm::dvec2 m_g;
	};
}
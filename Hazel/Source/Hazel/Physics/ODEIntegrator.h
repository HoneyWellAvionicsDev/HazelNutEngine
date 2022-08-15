#pragma once

#include "SystemState.h"

#include <glm/glm.hpp>


namespace Enyoo
{
	class ODEIntegrator
	{
	public:
		ODEIntegrator() = default;
		virtual ~ODEIntegrator() = default;

		void Start(SystemState& initalState, double dt);
		bool Step(SystemState& state);
		void Integrate(SystemState& state);
		void End();
	private:
		double m_dt;
	};
}

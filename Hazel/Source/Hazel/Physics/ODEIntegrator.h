#pragma once

#include "SystemState.h"


namespace Enyoo
{
	class ODEIntegrator
	{
	public:
		ODEIntegrator();
		virtual ~ODEIntegrator() = default;

		virtual void Start(SystemState& initalState, double dt);
		virtual bool Step(SystemState& state);
		virtual void Integrate(SystemState& state) = 0;
		void End();
	protected:
		double m_dt;
	};
}

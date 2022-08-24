#pragma once

#include "SystemState.h"


namespace Enyoo
{
	class ODEIntegrator
	{
	public:
		ODEIntegrator() = default;
		virtual ~ODEIntegrator() = default;

		virtual void Start(SystemState& initalState, double dt) = 0;
		virtual bool Step(SystemState& state) = 0;
		virtual void Integrate(SystemState& state) = 0;
		void End();
	protected:
		double m_dt;
	};
}

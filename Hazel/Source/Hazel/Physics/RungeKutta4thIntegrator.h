#pragma once

#include "ODEIntegrator.h"

namespace Enyoo
{
	class RungeKutta4thIntegrator : public ODEIntegrator
	{
	public:
		RungeKutta4thIntegrator() = default;
		virtual ~RungeKutta4thIntegrator() = default;

		virtual void Start(SystemState& initalState, double dt) override;
		virtual bool Step(SystemState& state) override;
		virtual void Integrate(SystemState& state) override;
	private:

	};
}

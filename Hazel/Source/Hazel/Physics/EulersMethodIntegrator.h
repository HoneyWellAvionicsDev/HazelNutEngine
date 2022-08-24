#pragma once

#include "ODEIntegrator.h"

namespace Enyoo
{
	class EulersMethodIntegrator : public ODEIntegrator
	{
	public:
		EulersMethodIntegrator() = default;
		virtual ~EulersMethodIntegrator() = default;

		virtual void Start(SystemState& initalState, double dt) override;
		virtual bool Step(SystemState& state) override;
		virtual void Integrate(SystemState& state) override;
	};
}

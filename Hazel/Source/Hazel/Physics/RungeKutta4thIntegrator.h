#pragma once

#include "ODEIntegrator.h"

namespace Enyoo
{
	class RungeKutta4thIntegrator : public ODEIntegrator
	{
	public:
		enum Stage
		{
			K1 = 0,
			K2,
			K3,
			K4,
			Done
		};

		RungeKutta4thIntegrator() = default;
		virtual ~RungeKutta4thIntegrator() = default;

		virtual void Start(SystemState& initalState, double dt) override;
		virtual bool Step(SystemState& state) override;
		virtual void Integrate(SystemState& state) override;
	private:
		Stage m_CurrentStage;
		Stage m_NextStage;

		SystemState m_InitialState;
		SystemState m_Accumulator;
	};
}

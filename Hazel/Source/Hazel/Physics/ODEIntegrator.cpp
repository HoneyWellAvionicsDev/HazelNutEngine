#include "hzpch.h"
#include "ODEIntegrator.h"


namespace Enyoo
{
	ODEIntegrator::ODEIntegrator()
		:m_dt(0.0)
	{
	}

	void ODEIntegrator::Start(SystemState& initalState, double dt)
	{
		m_dt = dt;
	}

	bool ODEIntegrator::Step(SystemState& state)
	{
		return true;
	}

	void ODEIntegrator::End()
	{
	}
}

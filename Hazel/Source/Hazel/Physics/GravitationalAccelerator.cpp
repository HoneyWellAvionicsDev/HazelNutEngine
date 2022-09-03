#include "hzpch.h"
#include "GravitationalAccelerator.h"

namespace Enyoo
{
	GravitationalAccelerator::GravitationalAccelerator()
		: m_g{0.0, -9.81}
	{
	}

	void GravitationalAccelerator::ApplyForce(SystemState& state)
	{	
		for (size_t i = 0; i < state.RigidBodyCount; i++)
		{
			state.Force[i] += state.Mass[i] * m_g;
		}
	}
}


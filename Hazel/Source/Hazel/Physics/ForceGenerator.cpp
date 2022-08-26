#include <hzpch.h>

#include "ForceGenerator.h"

namespace Enyoo
{
	void ForceGenerator::ApplyForce(SystemState& state)
	{
		for (size_t i = 0; i < state.RigidBodyCount; i++)
		{
			state.Force[i] += state.Mass[i] * glm::dvec2{ 0.0, -9.81 };
		}
	}
}
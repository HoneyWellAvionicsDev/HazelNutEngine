#include <hzpch.h>

#include "ForceGenerator.h"

namespace Enyoo
{
	void ForceGenerator::ApplyForce(SystemState& state)
	{
		for (int i = 0; i < state.RigidBodyCount; i++)
		{
			state.ApplyForce({ 0,0 }, { 0,-9.81 }, i); //test
		}
	}
}
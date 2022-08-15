#include <hzpch.h>

#include "ForceGenerator.h"

namespace Enyoo
{
	void ForceGenerator::ApplyForce(SystemState& state)
	{
		state.ApplyForce({ 0,0 }, { 0,-9.81 }, 0); //test
	}
}
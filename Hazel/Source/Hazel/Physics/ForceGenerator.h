#pragma once

#include "SystemState.h"

namespace Enyoo
{
	class ForceGenerator
	{
	public:
		ForceGenerator() = default;

		void ApplyForce(SystemState& state);
	//private:
		int m_Index;
	};
}